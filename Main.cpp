#include <pthread.h>
#include "crc64.h"
#include "define.h"



int Start(int subtype, bool deamon) {
  
    push = new pushout();
    push->setmqlb(mqlbsvr);
    getcmdFromzmq::getcmdFromzmqInit(THREADNUM);
    for (int i = 0; i < THREADNUM; i++) {
        docmd[i] = getcmdFromzmq::SvrList[i];
        push->setzpipe(&docmd[i]->zpipeOut);
    }

    mqlbsvr->setServerSocket(m_pServerSocket);
    push->Start();
    mqlbsvr->start();
    sleep(2);
    //LDB INIt
    LDB::appid = m_ConfigClient.GetID();
    std::set<std::string> topicList;
    topicList.clear();
    mqlbsvr->SettopicList(topicList);
    LDB::Init(topicList);  // DB init
    return r;
}



int OnConnect(CIOSocket *socket) {
    static int numconnect = 0;
    if (!m_bStarted)return -1;

    LOGI(":id=" << socket->GetID() << ",ip=" << socket->GetIP().c_str() << ",port=" << socket->GetPort());

    CPinfo *player = new CPinfo;
    int num = __sync_fetch_and_add(&numconnect, 1);
    player->id = num % THREADNUM;

    socket->SetUserData(player);
    return 0;
}


int OnReceive(CIOSocket *socket, char *buf, int size) {
   

    //MQ
    OldClientHead *pHead = (OldClientHead *) buf;
    try {
        switch (pHead->xyid) {
            case CMDT_CHECKACT: {
                break;
            }
           
            case CMDT_SENDACK: {
                ConsumAck(socket, buf + 4, pHead->len);
                break;
            }
            case CMDT_SENDACKPULL: {
                pullGetAck(socket, buf + 4, pHead->len);
                break;
            }
            case CMDT_SENDMSG: {
                GetMessg(socket, buf + 4, pHead->len);
                break;
            }
            case CMDT_REQREGCONSUMER: {
                PutMessg(socket, buf + 4, pHead->len);
                break;
            }
            case CMDT_REQREGCONSUMERPULL: {
                PutMessgPull(socket, buf + 4, pHead->len);
                break;
            }
            case CMDT_CLIREQMQCLOSETOPIC: {
                CliReqMQCloseTopic(socket, buf + 4, pHead->len);
                break;
            }

            default:
                break;
        }

    }
    catch (...) {
    }

    return 0;
}

int OnClose(CIOSocket *socket) {
    CPinfo *cpif = static_cast<CPinfo *>(socket->GetUserData());
    LOGI("Close appid:" << cpif->appid << ",groupid:" << cpif->groupid);

    std::map<CIOSocket *, std::map<int, std::set<std::string> > >::iterator sit = consumSocket2topic.find(socket);
    if (sit != consumSocket2topic.end()) {
        int appid = sit->second.begin()->first;
        std::set<std::string> &setTab = sit->second.begin()->second;

        std::set<std::string>::iterator itset = setTab.begin();
        for (; itset != setTab.end(); ++itset) {
            std::string grouptopicname = *itset;
            char *p = (char *) grouptopicname.c_str(), *q = NULL;
            while ((p = strstr(p + 1, "_")) != NULL) { q = p; }

            int t_Groupid = atoi(q + 1);
            JOB *job = new JOB(CMD_CLOSE, cpif->appid, t_Groupid, grouptopicname, NULL, 0);
            unsigned long addr1 = (unsigned long) job;

            int id = crc64::getcrc64(0, itset->c_str(), itset->length()) % THREADNUM;
            docmd[id]->zpipeIn.sneddata((char *) &addr1, sizeof(unsigned long));
        }

        setTab.clear();
        consumSocket2topic.erase(sit);
    }

    delete cpif;
    return 0;
}



int OnTimer(int nID) {
   

    return 0;
}



// IN MQ
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int GetMessg(CIOSocket *socket, char *buf, int size) {
    
    //get  msg from net;

    int t_Groupid = msg.nGroupID;
    if (t_Groupid == 0) t_Groupid = cpinfo->groupid;

    char strGroup[256];
    sprintf(strGroup, "_%d", t_Groupid);
    std::string groupTopicName = std::string(msg.szTopic) + std::string(strGroup);

    int topicid = mqlbsvr->getTopicId(msg.szTopic, t_Groupid);
    if (topicid < 0) {
        ReportErr Rerr;
        Rerr.nFlag = ReportErr::NOTOPIC;
        strcpy(Rerr.szTopic, msg.szTopic);
        SendProtocol(socket, Rerr);

        LOGI("### this mq have no topic: " << msg.szTopic << "t_Groupid:" << t_Groupid);
        return -1;
    }

    char buftmp[4096] = {0};
    sprintf(buftmp, "#_^%d#_^%s#_^%s#_^", t_Groupid, msg.szTopic, msg.szContent);

    JOB *job = new JOB(CMD_SET, cpinfo->appid, t_Groupid, groupTopicName, buftmp, strlen(buftmp));
    job->Productp = mqlbsvr->GetTopicAddr(topicid);
    unsigned long addr1 = (unsigned long) job;
    int id = crc64::getcrc64(0, groupTopicName.c_str(), groupTopicName.length()) % THREADNUM;
    docmd[id]->zpipeIn.sneddata((char *) &addr1, sizeof(unsigned long));

    // LOGI("#### Product topic:" << groupTopicName << ",addr=" << (unsigned long long) job->Productp);
    LOGI("#### Product topic:" << groupTopicName << ",msg:" << buftmp);
    return 0;
}

int PutMessg(CIOSocket *socket, char *buf, int size) {
    //get rrgCon;
    int topicGroupid = rrgCon.nGroupID;
    if (topicGroupid == 0) topicGroupid = cpinfo->groupid;

    char strGroup[256];
    sprintf(strGroup, "_%d", topicGroupid);
    std::string groupTopicName = std::string(rrgCon.szTopic) + std::string(strGroup);

    int topicid = mqlbsvr->getTopicId(rrgCon.szTopic, topicGroupid);
    if (topicid < 0) {
        rspconsum.nFlag = RespRegConsumer::NOTOPIC;
        LOGI("Have no this topic:" << rrgCon.szTopic << ",nGroupID:" << rrgCon.nGroupID);
        std::cout << "Out have no this topic:" << rrgCon.szTopic << ",nGroupID :" << rrgCon.nGroupID << std::endl;
        SendProtocol(socket, rspconsum);
        return -1;
    }

    rspconsum.nFlag = RespRegConsumer::SUCCESS;
    rspconsum.nAskID = rrgCon.nAskID;
    rspconsum.nGroupID = topicGroupid;
    sprintf(rspconsum.szTopic, "%s", rrgCon.szTopic);

    // re regist consumer
    if (consumSocket2topic.find(socket) != consumSocket2topic.end()) {
        std::map<int, std::set<std::string> > &setlist = consumSocket2topic.find(socket)->second;
        if (!setlist[cpinfo->appid].empty() &&
            setlist[cpinfo->appid].find(groupTopicName) != setlist[cpinfo->appid].end()) {
            LOGI("Consume this topic:" << rrgCon.szTopic << ",nGroupID:" << topicGroupid << " again...");
            return -1;
        }
    }
    Consuminfo *consume = new Consuminfo(std::string(rrgCon.szTopic), topicGroupid, cpinfo->appid, socket, m_nAppid);
    consumSocket2topic[socket][cpinfo->appid].insert(groupTopicName);
    SendProtocol(socket, rspconsum);

    cpinfo->topicMapTime[groupTopicName] = 0;
    cpinfo->topicMaptimes[groupTopicName] = 0;


    JOB *job = new JOB(CMD_GET, cpinfo->appid, topicGroupid, groupTopicName, NULL, 0);
    job->consumline = rrgCon.nOffset;
    job->Consump = consume->lineAddr;
    job->t = (void *) socket;

    unsigned long addr2 = (unsigned long) job;

    int id = crc64::getcrc64(0, groupTopicName.c_str(), groupTopicName.length()) % THREADNUM;
//    docmd[id]->zpipeIn.sneddata((char *) &addr2, sizeof(unsigned long));

    docmd[id]->zpipeConsumer.sneddata((char *) &addr2, sizeof(unsigned long));
    LOGI("### Consume topic:" << groupTopicName << ",offset:" << rrgCon.nOffset << ",addr="
                              << (unsigned long long) consume->lineAddr);

    return 0;
}

int PutMessgPull(CIOSocket *socket, char *buf, int size) {
    
    //get rrgCon;

    int topicGroupid = rrgCon.nGroupID;
    if (topicGroupid == 0) topicGroupid = cpinfo->groupid;

    char strGroup[256];
    sprintf(strGroup, "_%d", topicGroupid);
    std::string groupTopicName = std::string(rrgCon.szTopic) + std::string(strGroup);

    int topicid = mqlbsvr->getTopicId(rrgCon.szTopic, topicGroupid);
    if (topicid < 0) {
        rspconsum.nFlag = RespRegConsumer::NOTOPIC;
        LOGI("Have no this topic:" << rrgCon.szTopic << ",nGroupID:" << rrgCon.nGroupID);
        SendProtocol(socket, rspconsum);
        return -1;
    }

    rspconsum.nFlag = RespRegConsumer::SUCCESS;
    rspconsum.nAskID = rrgCon.nAskID;
    rspconsum.nGroupID = topicGroupid;
    sprintf(rspconsum.szTopic, "%s", rrgCon.szTopic);

    // re regist consumer
    if (consumSocket2topic.find(socket) != consumSocket2topic.end()) {
        std::map<int, std::set<std::string> > &setlist = consumSocket2topic.find(socket)->second;
        if (!setlist[cpinfo->appid].empty() &&
            setlist[cpinfo->appid].find(groupTopicName) != setlist[cpinfo->appid].end()) {
            LOGI("Consume this topic:" << rrgCon.szTopic << ",nGroupID:" << topicGroupid << " again...");
            return -1;
        }
    }

    Consuminfo *consume = new Consuminfo(std::string(rrgCon.szTopic), topicGroupid, cpinfo->appid, socket, m_nAppid);
    consumSocket2topic[socket][cpinfo->appid].insert(groupTopicName);
    SendProtocol(socket, rspconsum);

    cpinfo->topicMapTime[groupTopicName] = 0;
    cpinfo->topicMaptimes[groupTopicName] = 0;

    JOB *job = new JOB(CMD_GET, cpinfo->appid, topicGroupid, groupTopicName, NULL, 0);
    job->consumline = rrgCon.nOffset;
    job->Consump = consume->lineAddr;
    job->t = (void *) socket;
    job->pullflag = true;

    unsigned long addr2 = (unsigned long) job;
    int id = crc64::getcrc64(0, groupTopicName.c_str(), groupTopicName.length()) % THREADNUM;
    //docmd[id]->zpipeIn.sneddata((char *) &addr2, sizeof(unsigned long));
    docmd[id]->zpipeConsumer.sneddata((char *) &addr2, sizeof(unsigned long));
    LOGI("### PutMessgPull topic:" << groupTopicName << ",offset:" << rrgCon.nOffset << ",id=" << id);

    return 0;
}


int ConsumAck(CIOSocket *socket, char *buf, int size) {
   
   //get  sack;
    consumerTopicOffset[sack.nGroupID][cpinfo->appid][std::string(sack.szTopic)] = sack.nOffset;

    char strGroup[256];
    sprintf(strGroup, "_%d", sack.nGroupID);
    std::string groupTopicName = std::string(sack.szTopic) + std::string(strGroup);

    if (sack.nOffset < 0) return -1;

    if (time(0) - cpinfo->topicMapTime[groupTopicName] > 10 || cpinfo->topicMaptimes[groupTopicName] % 20 == 0) {
        JOB *job = new JOB(CMD_ACK, cpinfo->appid, sack.nGroupID, groupTopicName, NULL, 0); //fuck bug groupid
        job->consumline = sack.nOffset;
        unsigned long addr3 = (unsigned long) job;

        cpinfo->topicMapTime[groupTopicName] = time(0);
        int id = crc64::getcrc64(0, groupTopicName.c_str(), groupTopicName.length()) % THREADNUM;
//        docmd[id]->zpipeIn.sneddata((char *) &addr3, sizeof(unsigned long));
        docmd[id]->zpipeConsumer.sneddata((char *) &addr3, sizeof(unsigned long));
        LOGI("#### send save consumeLine:" << groupTopicName << ",id=" << id);
    }
    cpinfo->topicMaptimes[groupTopicName]++;
    return 0;
}


int pullGetAck(CIOSocket *socket, char *buf, int size) {
    CPinfo *cpinfo = static_cast<CPinfo *>(socket->GetUserData());
    SendAckPull sack;
    bistream bis;
    bostream bos;
    bis.attach(buf, size);
    bis >> sack;

    char strGroup[256];
    sprintf(strGroup, "_%d", sack.nGroupID);
    std::string groupTopicName = std::string(sack.szTopic) + std::string(strGroup);
    if (sack.nOffset >= 0 || sack.nOffset == sack.PULLAGIN) {
        if (sack.nOffset >= 0)
            consumerTopicOffset[sack.nGroupID][cpinfo->appid][std::string(sack.szTopic)] = sack.nOffset;

        JOB *job = new JOB(CMD_ACK_PULL, cpinfo->appid, sack.nGroupID, groupTopicName, NULL, 0);
        job->consumline = sack.nOffset;
        job->t = (void *) socket;
        unsigned long addr3 = (unsigned long) job;

        int id = crc64::getcrc64(0, groupTopicName.c_str(), groupTopicName.length()) % THREADNUM;
//        docmd[id]->zpipeIn.sneddata((char *) &addr3, sizeof(unsigned long));
        docmd[id]->zpipeConsumer.sneddata((char *) &addr3, sizeof(unsigned long));
        LOGI("#### pullGet save consumeLine: groupid:" << sack.nGroupID << ",topic:" << groupTopicName << ",id=" << id);
    } else {
        return -1;
    }


    return 0;
}


int CliReqMQCloseTopic(CIOSocket *socket, char *buf, int size) {
   
    //get rqclose;
    LOGI("Client ReqClose topic,appid:" << rqclose.appid << ",size:" << consumSocket2topic.size());

    std::ostringstream groupTopicName;
    groupTopicName << rqclose.sztopic << "_" << rqclose.nGroupID;
    std::map<int, std::set<std::string> > &setlist = consumSocket2topic[socket];
    std::set<std::string>::iterator it = setlist[cpinfo->appid].find(groupTopicName.str());
    if (it != setlist[cpinfo->appid].end())
        setlist[cpinfo->appid].erase(it);


    CIOSocket *sockettmp = NULL;
    std::map<CIOSocket *, std::map<int, std::set<std::string> > >::iterator sit = consumSocket2topic.begin();
    for (; sit != consumSocket2topic.end(); sit++) {
        if (sit->second.begin()->first == rqclose.appid) {
            sockettmp = sit->first;
            break;
        }
    }
    if (sit == consumSocket2topic.end()) return -1;

    if (rqclose.flag == rqclose.all) {
        int appid = sit->second.begin()->first;
        std::set<std::string> &setTab = sit->second.begin()->second;
        std::set<std::string>::iterator itset = setTab.begin();
        for (; itset != setTab.end(); ++itset) {
            JOB *job = new JOB(CMD_CLOSE, rqclose.appid, rqclose.nGroupID, *itset, NULL, 0);
            job->LBAskid = rqclose.Askid;
            unsigned long addr1 = (unsigned long) job;
            int id = crc64::getcrc64(0, itset->c_str(), itset->length()) % THREADNUM;
//            docmd[id]->zpipeIn.sneddata((char *) &addr1, sizeof(unsigned long));
            docmd[id]->zpipeConsumer.sneddata((char *) &addr1, sizeof(unsigned long));

            setTab.clear();
            consumSocket2topic.erase(sit);
            return sockettmp->Close();
        }
    } else if (rqclose.flag == rqclose.one) {
        ostringstream osstring;
        osstring << rqclose.sztopic << "_" << rqclose.
                nGroupID;
        std::string topic = osstring.str();

        JOB *job = new JOB(CMD_CLIENT_CLOSE_TOPIC, rqclose.appid, rqclose.nGroupID, topic, NULL, 0);
        job->LBAskid = rqclose.Askid;
        unsigned long addr1 = (unsigned long) job;
        int id = crc64::getcrc64(0, topic.c_str(), topic.length()) % THREADNUM;
//        docmd[id]->zpipeIn.sneddata((char *) &addr1, sizeof(unsigned long));
        docmd[id]->zpipeConsumer.sneddata((
                                                  char *) &addr1, sizeof(unsigned long));
        LOGI("###1 Client Close_topic:" << topic << ",appid:" << rqclose.appid << ",groupid:" << rqclose.nGroupID);
        return 0;
    }


    return -1;
}