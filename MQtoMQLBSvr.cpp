//
// Created by dongbo01 on 5/16/17.
//
#include <assert.h>
#include "MQtoMQLBSvr.h"
#include <list>

MQtoMQLBSvr::MQtoMQLBSvr(int _appid, int _groupid) {
    m_pServerSocket = NULL;
    pthread_mutex_init(&lock, 0);
    pthread_spin_init(&topicidlock, 0);
    pthread_spin_init(&topicListlock, 0);
    numTopic = 0;
    appid = _appid;
    groupid = _groupid;
    eventBase = event_base_new();
    pipe(cmdfd);
    bevpipe = bufferevent_socket_new(eventBase, cmdfd[0], BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bevpipe, ReadEvent, NULL, NULL, (void *) this);
    bufferevent_enable(bevpipe, EV_READ);
}

void MQtoMQLBSvr::setServerSocket(CMultiServerSocket *ServerSocket) {
    m_pServerSocket = ServerSocket;
}

int MQtoMQLBSvr::Connect(char *pRemoteIp, unsigned short nPort) {
    bev = bufferevent_socket_new(eventBase, -1, BEV_OPT_CLOSE_ON_FREE);
    assert(bev);

    bufferevent_setcb(bev, ReadEvent, NULL, CloseEvent, (void *) this);
    bufferevent_enable(bev, EV_WRITE);
    bufferevent_enable(bev, EV_READ);

    struct sockaddr_in connect_to_addr;
    memset(&connect_to_addr, 0, sizeof(connect_to_addr));
    connect_to_addr.sin_family = AF_INET;
    connect_to_addr.sin_addr.s_addr = inet_addr(pRemoteIp);
    connect_to_addr.sin_port = htons(nPort); //nPort

    std::cout << "connect ip:" << pRemoteIp << ":" << nPort << " ..." << std::endl;

    if (bufferevent_socket_connect(bev, (struct sockaddr *) &connect_to_addr, sizeof(connect_to_addr)) < 0) {
        perror("bufferevent_socket_connect");
        return -1;
    } else {
        std::cout << "connect OK" << std::endl;
        sendReg();
        static int flag = 1;
        if (flag) {
            flag = 0;

            TimerEvent = event_new(eventBase, -1, 0, OnTimer, (void *) this);
            event_base_dispatch(eventBase);
        }
    }

    return 0;
}

int MQtoMQLBSvr::Connect() {
    static unsigned int i = 0;

    if (mqlblist.size() <= 0) {
        std::cout << "Unkown MQLBSvr Addr " << std::endl;
        LOGI("Unkown MQLBSvr Addr ");
        return -1;
    }

    int id = i % mqlblist.size();
    char *pRemoteIp = (char *) mqlblist[id].ip.c_str();
    unsigned short nPort = mqlblist[id].port;

    sleep(5);
    Connect(pRemoteIp, nPort);

    i++;
    return 0;
}


void MQtoMQLBSvr::CloseEvent(struct bufferevent *bev, short events, void *arg) {

    MQtoMQLBSvr *me = (MQtoMQLBSvr *) arg;
    if (events & BEV_EVENT_EOF) {
        std::cout << "#connection Close:" << strerror(errno) << std::endl;
        bufferevent_free(bev);
        me->Connect();

    } else if (events & BEV_EVENT_ERROR) {
        std::cout << "#Got an error on the connection:" << strerror(errno) << std::endl;
        bufferevent_free(bev);
        me->Connect();
    }
}


void MQtoMQLBSvr::ReadEvent(bufferevent *bev, void *arg) {
    static staticRec *s = NULL;
    if (s == NULL) s = new staticRec();

    MQtoMQLBSvr *me = (MQtoMQLBSvr *) arg;
    int len = bufferevent_read(bev, s->m_pRecvBuf + s->m_nRecvBufUsed, s->m_nBufSize - s->m_nRecvBufUsed);

    if (len > 0) {
        s->m_nRecvBufUsed += len;
        int left = s->m_nRecvBufUsed;
        int used = 0;
        while (left) {
            int pkgsize = me->FliterStream(s, s->m_pRecvBuf + used, left);
            if (pkgsize > 0) {
                me->doRec(s->m_pRecvBuf + used, pkgsize);
            } else if (pkgsize == 0) {
                break;
            }

            used += pkgsize;
            left -= pkgsize;
        }

        s->m_nRecvBufUsed -= used;
        if (s->m_nRecvBufUsed > 0)
            memmove(s->m_pRecvBuf, s->m_pRecvBuf + used, s->m_nRecvBufUsed);
    }

}


int MQtoMQLBSvr::FliterStream(staticRec *s, char *buf, int size) {
    if (size < OLDCLIENTHEADLEN)
        return 0;

    unsigned short paksize = (*(unsigned short *) buf) + OLDCLIENTHEADLEN;
    if (paksize > 4096) {
        return -1;
    }
    if (paksize <= size)
        return paksize;
    else
        return 0;

}

int MQtoMQLBSvr::doRec(char *buf, int len) {
    OldClientHead *head = (OldClientHead *) buf;
    char *buf2 = (buf + 4);

//    std::cout << "XYID :" << head->xyid << std::endl;
    switch (head->xyid) {
        case Protocol::V32::MQLBSvr::MQ::CMDT_CHECKACT: {
            break;
        }
        case Protocol::V32::MQLBSvr::MQ::CMDT_RESPREG: {
            if (!MQLBSvrRegRsp(buf2, head->len)) {
                timeval tv;
                tv.tv_sec = 100;
                tv.tv_usec = 0;
                event_add(TimerEvent, &tv);
            }
            break;
        }
        case Protocol::V32::MQLBSvr::MQ::CMDT_REQCREATETOPIC: {
            RecvCreateTopic(buf2, head->len);
            break;
        }
        case Protocol::V32::MQLBSvr::MQ::CMDT_RESPTOPICLIST: {
            RecTopicListFromMQLBSvr(buf2, head->len);
            break;
        }
        case Protocol::V32::MQLBSvr::MQ::CMDT_REQTOPICLIST: {
            break;
        }
        case Protocol::V32::MQLBSvr::MQ::CMDT_REQMQCLOSETOPIC: {
            LBMQCloseClient(buf2, head->len);
            break;
        }
        case Protocol::V32::MQLBSvr::MQ::CMDT_ERROT_INFO : {
            Protocol::V32::MQLBSvr::MQ::ErrorInfo err;
            bistream bis;
            bis.detach();
            bis.attach(buf2, head->len);
            bis >> err;
            std::cout << err.message << std::endl;

            break;
        }
    }
    return 0;

}


void *MQtoMQLBSvr::Run(void *arg) {
    MQtoMQLBSvr *lb = static_cast<MQtoMQLBSvr *>(arg);
    lb->topicListReadFromdisk();
    lb->Connect();

    return NULL;
}

int MQtoMQLBSvr::start() {
    pthread_t tid;
    pthread_create(&tid, NULL, Run, (void *) this);

    return 0;
}


int MQtoMQLBSvr::sendBalance() {
    static bool flag = false;
    if (!flag) {
        sg_init(1);
        sg_drop_privileges();
        flag = true;
    }

    int loadnum = 0;
    size_t ps_size;
    sg_process_stats *ps = sg_get_process_stats(&ps_size);
    for (size_t x = 0; x < ps_size; x++) {
        if (getpid() == ps->pid) {
            loadnum = static_cast<int>(ps->proc_resident * (float) ps->cpu_percent / 1024 / 10);
            break;
        }
        ps++;
    }


    bostream bo;
    char s[2048];
    OldClientHead *Head = (OldClientHead *) s;
    bo.attach(s + 4, 2000);
    Protocol::V32::MQLBSvr::MQ::ReportBlance rpBlance;
    rpBlance.nBlance = loadnum;

    bo << rpBlance;
    Head->xyid = Protocol::V32::MQLBSvr::MQ::CMDT_REPORTBLANCE;
    Head->len = bo.length();

    //  bufferevent_write(bev, s, bo.length() + 4);

    SendMess(s, bo.length() + 4);

    LOGI("SEND load :" << rpBlance.nBlance);
    std::cout << "#### send load =" << (unsigned long) rpBlance.nBlance << std::endl;

    return 0;
}

int MQtoMQLBSvr::sendReg() {
    bostream bo;
    char s[2048];
    OldClientHead *Head = (OldClientHead *) s;
    bo.attach(s + 4, 2000);

    Protocol::V32::MQLBSvr::MQ::ReqReg rqreg;
    rqreg.nAppID = appid;

    bo << rqreg;
    Head->xyid = rqreg.XY_ID;
    Head->len = bo.length();

    //bufferevent_write(bev, s, bo.length() + 4);
    SendMess(s, bo.length() + 4);

    LOGI("send nAppID:" << appid);
    std::cout << "send nAppID:" << appid << std::endl;
    return 0;
}

int MQtoMQLBSvr::MQLBSvrRegRsp(char *buf, int len) {
    Protocol::V32::MQLBSvr::MQ::RespReg pr;
    bistream bis;
    bis.detach();
    bis.attach(buf, len);
    bis >> pr;

    if (pr.nFlag == 0) {
        sendBalance();
        reqTopicList();
    }

    LOGI("Reg MQLBSvr, flag:" << pr.nFlag);
    std::cout << "Reg MQLBSvr , flag:" << pr.nFlag << std::endl;
    return pr.nFlag;
}

int MQtoMQLBSvr::reqTopicList() {
    bostream bo;
    bo.detach();
    char s[2048];
    OldClientHead *Head = (OldClientHead *) s;
    bo.attach(s + 4, 2000);

    Protocol::V32::MQLBSvr::MQ::ReqTopicList reqTList;
    bo << reqTList;

    Head->xyid = reqTList.XY_ID;
    Head->len = bo.length();
    // bufferevent_write(bev, s, bo.length() + 4);


    SendMess(s, bo.length() + 4);

    LOGI("request TopicList from MQLBSvr");
    std::cout << "request TopicList from MQLBSvr " << std::endl;
    return 0;
}

int MQtoMQLBSvr::rspTopicList() {
    return 0;
}


int MQtoMQLBSvr::MasterCreateTopic(char szTopic[256], int nContentSize, int nGroupID) {
    int flag = 1;
    std::string topicName = szTopic;
    int topicid = getTopicId(topicName, nGroupID);

    if (topicid < 0 && !topicName.empty()) {
        topicid = setTopicId(topicName, nGroupID);

        pthread_spin_lock(&topicListlock);
        TopicList[topicid].TopicName = szTopic;
        TopicList[topicid].groupid = nGroupID;
        TopicList[topicid].StartDISKLineNum = 0;
        TopicList[topicid].EndDISKLineNum = 0;
        TopicList[topicid].createMMAP(appid);
        pthread_spin_unlock(&topicListlock);

        LOGI("create new Topic:" << szTopic << " nGroupID:" << nGroupID);
        std::cout << "### Create new Topic:" << szTopic << " nGroupID:" << nGroupID << std::endl;
        flag = 0;
    } else if (topicid >= 0) {
        flag = 0;
        LOGI(" Have this Topic:" << topicName);
        std::cout << "Aready have this Topic:" << topicName << std::endl;
    }
    return flag;
}


int MQtoMQLBSvr::RecTopicListFromMQLBSvr(char *buf, int len) {
    bistream bis;
    bis.attach(buf, len);

    Protocol::V32::MQLBSvr::MQ::RespTopicList rspTlist;
    bis >> rspTlist;

    //std::cout << "RecTopicListFromMQLBSvr" <<std::endl;

    MasterCreateTopic(rspTlist.szTopic, 0, rspTlist.nGroupID);
    return 0;

}

int MQtoMQLBSvr::RecvCreateTopic(char *buf, int len) {
    bistream bis;
    bis.attach(buf, len);

    Protocol::V32::MQLBSvr::MQ::ReqCreateTopic rqctc;
    bis >> rqctc;

    std::cout << "recv Topic list:" << rqctc.szTopic << " " << rqctc.nAskID << " " << rqctc.nGroupID << std::endl;

    int flag = MasterCreateTopic(rqctc.szTopic, rqctc.nContentSize, rqctc.nGroupID);

    if (!flag) {
        Protocol::V32::MQLBSvr::MQ::RespCreateTopic rspceatTopic;
        rspceatTopic.nFlag = rspceatTopic.SUCCESS;
        rspceatTopic.nAskID = rqctc.nAskID;
        rspceatTopic.nGoupid = rqctc.nGroupID;
        sprintf(rspceatTopic.szTopic, "%s", rqctc.szTopic);

        bostream bo;
        bo.detach();
        char s[2048];
        OldClientHead *Head = (OldClientHead *) s;
        bo.attach(s + 4, 2000);

        bo << rspceatTopic;
        Head->xyid = rspceatTopic.XY_ID;
        Head->len = bo.length();
        // bufferevent_write(bev, s, bo.length() + 4);


        SendMess(s, bo.length() + 4);
    }

    return flag;
}

int MQtoMQLBSvr::setTopicId(std::string topic, int groupid) {
    int find = -1;
    int id = -1;
    pthread_spin_lock(&topicidlock);
//    std::map<int, std::map<std::string, int> >::iterator git = topicMapId.find(groupid);
//    if (git == topicMapId.end()) {
//        find = 0;
//    } else {
//        std::map<std::string, int>::iterator tit = git->second.find(topic);
//        if (tit == git->second.end()) {
//            find = 0;
//        }
//    }
//    if (!find) {
//        numTopic++;
//        id = numTopic;
//        topicMapId[groupid][topic] = id;
//
//
//    }

    std::cout << "set topic:" << topic << ",groupid:" << groupid << std::endl;
    id = topicMapId[groupid][topic];
    if (id == 0) {
        topicMapId[groupid][topic] = ++numTopic;
        id = numTopic;
    }

    pthread_spin_unlock(&topicidlock);

    return id;
}

int MQtoMQLBSvr::getTopicId(std::string topic, int groupid) {
    int id = -1;
    pthread_spin_lock(&topicidlock);
    std::map<int, std::map<std::string, int> >::iterator git = topicMapId.find(groupid);
    if (git != topicMapId.end()) {
        std::map<std::string, int>::iterator tit = git->second.find(topic);
        if (tit != git->second.end()) {
            id = tit->second;
        }
    }
    pthread_spin_unlock(&topicidlock);
    return id;
}

int MQtoMQLBSvr::SetMQLbSvripport(std::string ip_, int port_) {
    pthread_spin_lock(&topicidlock);
    MQLBSvrInfo node;
    node.ip = ip_;
    node.port = port_;
    mqlblist.push_back(node);
    pthread_spin_unlock(&topicidlock);
    return 0;
}

void MQtoMQLBSvr::OnTimer(int fd, short event, void *arg) {
    MQtoMQLBSvr *me = (MQtoMQLBSvr *) arg;
    me->sendBalance();
    timeval tv;
    tv.tv_sec = 100;
    tv.tv_usec = 0;
    event_add(me->TimerEvent, &tv);
}


void MQtoMQLBSvr::topicListReadFromdisk() {
    std::ifstream f("./topiclist");
    if (f.is_open()) {
        std::string s;
        while (getline(f, s)) {
            char grouptopic[1024];
            sprintf(grouptopic, "%s", s.c_str());

            int nGroupID = -1;
            std::string topic;
            char *p = strtok(grouptopic, "_");
            if (p) nGroupID = atoi(p);
            p = strtok(NULL, "_");
            if (p) topic = p;

            MasterCreateTopic((char *) topic.c_str(), 1, nGroupID);
            LOGI("load topic: groupid=" << nGroupID << ",topic=" << topic);
        }
        f.close();
    }
}

void MQtoMQLBSvr::SettopicList(std::set<std::string> &topicList) {

    pthread_spin_lock(&topicListlock);
    for (int i = 1; i <= numTopic; i++) {
        std::string topic = TopicList[i].TopicName;
        int nGroupID = TopicList[i].groupid;

        char tmp[256] = {0};
        sprintf(tmp, "%s_%d", topic.c_str(), nGroupID);
        topicList.insert(std::string(tmp));
    }
    pthread_spin_unlock(&topicListlock);
}

char *MQtoMQLBSvr::GetTopicAddr(int id) {
    if (id > 0 && id <= numTopic) return TopicList[id].lineAddr;
    return NULL;
}

int MQtoMQLBSvr::LBMQCloseClient(char *buf, int len) {
    bistream bis;
    bis.attach(buf, len);
    Protocol::V32::MQLBSvr::MQ::LBReqMQCloseTopic reqclose;
    bis >> reqclose;

    lbCMDCli *lbcl = new lbCMDCli();
    lbcl->XYID = lbcl->CMD_LBTOCLI_CLOSE;

    Protocol::V32::MQSvr::CliReqMQCloseTopic *rqclose = new Protocol::V32::MQSvr::CliReqMQCloseTopic();
    rqclose->appid = reqclose.nAppID;
    rqclose->nGroupID = reqclose.nGroupid;
    rqclose->flag = reqclose.flag;
    rqclose->Askid = reqclose.Askid;
    sprintf(rqclose->sztopic, "%s", reqclose.szTopic);

    if (reqclose.flag == reqclose.all) {
        rqclose->flag = rqclose->all;
    } else if (reqclose.one == reqclose.flag) {
        rqclose->flag = rqclose->one;
    }

    LOGI("###1 Close_topic:" << rqclose->sztopic << ",appid=" << rqclose->appid << ",groupid:" << rqclose->nGroupID);
    lbcl->addr = (void *) rqclose;
    m_pServerSocket->PushUserEvent(0, (void *) lbcl);

    return 0;
}


int MQtoMQLBSvr::SendMess(char *buf, int len) {
    pthread_mutex_lock(&lock);
    int f = bufferevent_write(bev, buf, len);
    pthread_mutex_unlock(&lock);

    return f;
}


