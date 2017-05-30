//
// Created by dongbo01 on 5/23/17.
//
#include <cstring>
#include <cstdlib>
#include "pushout.h"
#include <list>
//#include "common/IOSocket.h"

#include <assert.h>

int pushout::setzpipe(zmqpipe *pipe) {
    zpipelist.push_back(pipe);
    return zpipelist.size();
}

void pushout::push() {
    while (true) {
        std::list<int> flaglist;
        for (int j = 0; j < zpipelist.size(); j++) {
            unsigned long buf[1024];
            int len = zpipelist[j]->getdata((char *) buf, 1024 * 8);
            if (len < 0) {//loop push
                flaglist.push_back(1);
            } else {
                for (unsigned int i = 0; i < len / sizeof(unsigned long); i++) {
                    JOB *eTask = (JOB *) buf[i];

//                    bistream bis;
//                    bis.attach(eTask->p, eTask->slen);
//                    Protocol::V32::MQSvr::SendMsg msg;
//                    bis >> msg;
//
//                    std::cout << "xyid:" << eTask->xyid << ",msg=" << msg.szContent << std::endl;
//
//                    char mess[4096];
//                    OldClientHead *head = (OldClientHead *) mess;
//                    head->xyid = Protocol::V32::MQSvr::SendMsg::XY_ID;
//                    head->len = eTask->slen;
//                    memcpy(mess + 4, eTask->p, eTask->slen);
//
//                    CIOSocket *Socket = static_cast<CIOSocket *>(eTask->t);
//                    assert(Socket != NULL);
//                    Socket->AsynSend(mess, head->len + 4);

                    std::cout <<time(0) << " xyid="<<eTask->xyid<<",topic:" << eTask->topic <<",value:"<<eTask->p <<",len:" << eTask->slen << std::endl;

                    delete eTask;
                }
            }
        }//for

        if (flaglist.size() == zpipelist.size())
            zpipelist[0]->setTimeOut(1000);
        else
            zpipelist[0]->setTimeOut(0);

        flaglist.clear();
    }
}

void *pushout::Run(void *arg) {
    pushout *me = static_cast<pushout *>(arg);
    me->push();


    return NULL;
}

int pushout::Start() {
    pthread_t tid;
    pthread_create(&tid, NULL, Run, (void *) this);
    return 0;
}
