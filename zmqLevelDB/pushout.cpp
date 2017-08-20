//
// Created by dongbo01 on 5/23/17.
//
#include <cstring>
#include <cstdlib>
#include <list>
#include <assert.h>
#include "pushout.h"
#include "common/IOSocket.h"
#include "common/Log.h"
#include "getcmdFromzmq.h"
#include "define.h"

#include "Svr.h"

int pushout::setzpipe(zmqpipe *pipe) {
    zpipelist.push_back(pipe);
    return zpipelist.size();
}

void pushout::push() {

    std::list<int> flaglist;
    int timeOut = 1;
    zmq_pollitem_t items[10];

    for (int i = 0; i < THREADNUM; i++) {
        items[i].socket = zpipelist[i]->getPipEnd();
        items[i].events = ZMQ_POLLIN;
    }

    while (true) {
        int rc = -1;
        int num = zmq_poll(items, THREADNUM, timeOut);
        if (num) {
            for (int i = 0; i < THREADNUM; ++i) {
                unsigned long buf;
                if (items[i].revents > 0) {
                    zmq_recv(items[i].socket, (char *) &buf, sizeof(unsigned long), 0);
                    JOB *eTask = (JOB *) buf;

                    if (eTask->xyid == CMD_SET) {
                        FUNC_CMD_SET(eTask);
                    } else if (eTask->xyid == CMD_GET) {
                        FUNC_CMD_GET(eTask);
                    } else if (eTask->xyid == CMD_CLOSE) {
                        FUNC_CMD_CLOSE(eTask);
                    } else if (eTask->xyid == CMD_ACK) {
                        FUNC_CMD_ACK(eTask);
                    } else if (eTask->xyid == CMD_CLIENT_CLOSE_TOPIC) {
                        FUNC_CMD_CLIENT_CLOSE_TOPIC(eTask);
                    } else if (eTask->xyid == CMD_LB_CLOSE_TOPIC) {
                        FUNC_CMD_LB_CLOSE_TOPIC(eTask);
                    } else if (eTask->xyid == CMD_ACK_PULL) {
                        FUNC_CMD_ACK_PULL(eTask);
                    }

                    delete eTask;
                }
            }
            timeOut = 0;
        } else {
            timeOut = 1000;
        }
    }
}

void *pushout::Run(void *arg) {
    prctl(PR_SET_NAME, "PUSH_THREAD");
    pushout *me = static_cast<pushout *>(arg);
    me->push();

    return NULL;
}

int pushout::Start() {
    pthread_t tid;
    pthread_create(&tid, NULL, Run, (void *) this);
    return 0;
}

void pushout::setmqlb(MQtoMQLBSvr *mqlbSvr_) {
    mqlbSvr = mqlbSvr_;
}


void pushout::FUNC_CMD_SET(JOB *eTask) {}

void pushout::FUNC_CMD_GET(JOB *eTask) {
    Protocol::V32::MQSvr::SendMsg msg; // sprintf(buftmp, "#_^%d#_^%s#_^%s#_^", msg.nGroupID, msg.szTopic, msg.szContent);

    std::string strtmp(eTask->p);
    int i1 = strtmp.find("#_^", 0);
    int i2 = strtmp.find("#_^", i1 + 1);
    int i3 = strtmp.find("#_^", i2 + 1);
    int i4 = strtmp.find("#_^", i3 + 1);

    std::string nGroupID;
    std::string szTopic;
    std::string szContent;

    nGroupID = strtmp.substr(i1 + 3, i2 - i1 - 3);
    szTopic = strtmp.substr(i2 + 3, i3 - i2 - 3);
    szContent = strtmp.substr(i3 + 3, i4 - i3 - 3);

    msg.nGroupID = atoi(nGroupID.c_str());
    msg.nOffset = eTask->consumline;
    sprintf(msg.szTopic, "%s", szTopic.c_str());
    sprintf(msg.szContent, "%s", szContent.c_str());


    int l = sendMess(msg, eTask);
    LOGI("### AsyncSend topic:" << msg.szTopic << ",Groupid:" << msg.nGroupID << ",offset=" << msg.nOffset
                                << ",Content:" << msg.szContent << ",len=" << l);
}

void pushout::FUNC_CMD_CLOSE(JOB *eTask) {}

void pushout::FUNC_CMD_ACK(JOB *eTask) {}


void pushout::FUNC_CMD_CLIENT_CLOSE_TOPIC(JOB *eTask) {
    Protocol::V32::MQSvr::RespClientCloseTopic rspclose;
    rspclose.appid = eTask->appid;
    rspclose.nGroupID = eTask->groupid;
    rspclose.Askid = eTask->LBAskid;
    rspclose.offset = eTask->offset;

    int len = strlen(eTask->topic);
    while (len > 0) {
        if (eTask->topic[len] == '_') {
            eTask->topic[len] = '\0';
            break;
        } else
            len--;
    }
    sprintf(rspclose.szTopic, "%s", eTask->topic);

    if (eTask->flag == eTask->one) {
        rspclose.flag = rspclose.one;
    } else if (eTask->flag == eTask->all) {
        rspclose.flag = rspclose.all;
    }

    int l = sendMess(rspclose, eTask);
    LOGI("### AsyncSend,topic:" << rspclose.szTopic << ",len=" << l << ",offset:" << rspclose.offset);
}

void pushout::FUNC_CMD_LB_CLOSE_TOPIC(JOB *eTask) {
    Protocol::V32::MQLBSvr::Client::RespLBClostTopic rspclose;
    rspclose.nAppid = eTask->appid;
    rspclose.nGroupid = eTask->groupid;
    rspclose.Askid = eTask->LBAskid;
    rspclose.offset = eTask->offset;

    if (eTask->flag == eTask->one) {
        rspclose.flag = rspclose.one;
    } else if (eTask->flag == eTask->all) {
        rspclose.flag = rspclose.all;
    }

    mqlbSvr->putMessToLB(rspclose);
}

void pushout::FUNC_CMD_ACK_PULL(JOB *eTask) {
    LOGI(":" << eTask->topic);

    Protocol::V32::MQSvr::SendMsg msg; //sprintf(buftmp, "#_%d#_%s#_%s#_", msg.nGroupID, msg.szTopic, msg.szContent);
    if (strlen(eTask->p) > 0) {
        std::string strtmp(eTask->p);
        int i1 = strtmp.find("#_^", 0);
        int i2 = strtmp.find("#_^", i1 + 1);
        int i3 = strtmp.find("#_^", i2 + 1);
        int i4 = strtmp.find("#_^", i3 + 1);

        std::string nGroupID;
        std::string szTopic;
        std::string szContent;

        nGroupID = strtmp.substr(i1 + 3, i2 - i1 - 3);
        szTopic = strtmp.substr(i2 + 3, i3 - i2 - 3);
        szContent = strtmp.substr(i3 + 3, i4 - i3 - 3);

        msg.nGroupID = atoi(nGroupID.c_str());
        msg.nOffset = eTask->consumline;
        sprintf(msg.szTopic, "%s", szTopic.c_str());
        sprintf(msg.szContent, "%s", szContent.c_str());
    } else {
        int len = strlen(eTask->topic);
        while (len > 0) {
            if (eTask->topic[len] == '_') {
                eTask->topic[len] = '\0';
                break;
            } else
                len--;
        }
        sprintf(msg.szTopic, "%s", eTask->topic);
        msg.szContent[0] = '\0';
        msg.nGroupID = eTask->groupid;
    }

    msg.nOffset = eTask->consumline;
    int l = sendMess(msg, eTask);

    LOGI("### AsyncSend topic:" << msg.szTopic << ",Groupid:" << msg.nGroupID << ",offset=" << msg.nOffset
                                << ",Content:" << msg.szContent << ",len=" << l);
}
