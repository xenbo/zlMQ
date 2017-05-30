//
// Created by dongbo on 5/16/17.
//

#include "getcmdFromzmq.h"
#include <cassert>

int  getcmdFromzmq::numSvr = 0;
std::vector<getcmdFromzmq *> getcmdFromzmq::SvrList = getcmdFromzmq::createlist();

void getcmdFromzmq::getcmdFromzmqInit(int num) {
    numSvr = num;
    for (int i = 0; i < num; i++) {
        getcmdFromzmq *mev = new getcmdFromzmq(i);
        SvrList.push_back(mev);

        pthread_t tid;
        pthread_create(&tid, NULL, Run, (void *) mev);
    }
}

getcmdFromzmq::getcmdFromzmq(int _id) {
    id = _id;
}

getcmdFromzmq::~getcmdFromzmq() {

}

void *getcmdFromzmq::Run(void *arg) {
    getcmdFromzmq *me = (getcmdFromzmq *) arg;
    while (true) {
        unsigned long buf[1024];
        int len = me->zpipeIn.getdata((char *) buf, 1024 * 8);

        if (len < 0) {//loop push
            me->GetcmdtimeOut();
        } else {
            for (unsigned int i = 0; i < len / sizeof(unsigned long); i++) {
                JOB *eTask = (JOB *) buf[i];
                me->GetcmdtimeNOut(eTask);
            }//for
        }
    }

    return NULL;
}


int getcmdFromzmq::GetcmdtimeNOut(JOB *eTask) {
//  std::cout << "XXXXXXXXXXXXXx  in:"<<eTask->topic <<",xiyd:"<< eTask->xyid<< std::endl;

    char tmp[300];
    sprintf(tmp, "%s_%d", eTask->topic, eTask->groupid);
    std::string topic = tmp;

    opleveldb *opdb = doCmdFindDB[topic][eTask->appid];
    if (!opdb) {
        opdb = new opleveldb(eTask->appid, eTask->groupid);
        opdb->opinitdb(topic);
        doCmdFindDB[topic][eTask->appid] = opdb;
    }

    if (eTask->xyid == 0) {//set
        opdb->opput(topic, std::string(eTask->p, eTask->slen));

        //loop push
        opPushlist2[topic] = opPushlist1[topic];
        for (std::map<std::string, std::map<int, opleveldb *> >::iterator iter = opPushlist2.begin();
             iter != opPushlist2.end();) {
            std::string data;
            std::string optopic = iter->first;
            std::map<int, opleveldb *> &Opmap = iter->second;

            for (std::map<int, opleveldb *>::iterator opiter = Opmap.begin(); opiter != Opmap.end();) {
                int consumAppid = opiter->first;
                opleveldb *opdbloop = opiter->second;

                if (opdbloop) {
                    int flag = opdbloop->opget(optopic, &data);
                    if (flag >= 0) {
                        JOB *job = new JOB(3, consumAppid, 0, optopic, (char *) data.c_str(), data.length());
                        job->t = opdbloop->t;

                        unsigned  long addr = (unsigned long)job;
                        zpipeOut.sneddata((char*)&addr,sizeof(unsigned long));
                    } else {
                        Opmap.erase(opiter++);
                        continue;
                    }
                }// end if
                opiter++;
            } //for opiter

            if (Opmap.empty())
                opPushlist2.erase(iter++);
            else
                iter++;
        }//loop push end
    } // end if set 0
    else if (eTask->xyid == 1) {//get ,add pushTable
        opleveldb *opgetdb = doCmdFindDB[topic][eTask->appid];
        if (!opgetdb) {
            opgetdb = new opleveldb(eTask->appid, eTask->groupid);
            opgetdb->opinitdb(topic, (long long) eTask->consumline);
            doCmdFindDB[topic][eTask->appid] = opgetdb;
        } else {
            opgetdb->setConsumerLine(topic, (long long) eTask->consumline);
        }

        opgetdb->t = eTask->t;
        std::cout << "Reg consum:" << eTask->topic << ",offset:" << eTask->offset << std::endl;

        opPushlist1[topic][eTask->appid] = opgetdb;
    } //if get 1

    delete eTask;

    return 0;
}

int getcmdFromzmq::GetcmdtimeOut() {
//    std::cout << "XXXXXXXXXXXXXx  tiomeOut" << std::endl;
//  loop push

    std::list<int> flaglist; flaglist.clear();
    for (std::map<std::string, std::map<int, opleveldb *> >::iterator iter = opPushlist1.begin();
         iter != opPushlist1.end(); iter++) {
        std::string data;
        std::string optopic = iter->first;
        std::map<int, opleveldb *> &Opmap = iter->second;

        for (std::map<int, opleveldb *>::iterator opiter = Opmap.begin(); opiter != Opmap.end(); opiter++) {
            int consumAppid = opiter->first;
            opleveldb *opdbloop = opiter->second;

            if (opdbloop) {
                int flag = opdbloop->opget(optopic, &data);
                if (flag >= 0) {
                    JOB *job = new JOB(4, consumAppid, 0, optopic, (char *) data.c_str(), data.length());
                    job->t = opdbloop->t;

                    unsigned  long addr = (unsigned long)job;
                    zpipeOut.sneddata((char*)&addr,sizeof(unsigned long));
                    flaglist.push_back(1);
                }
            }
        }
    }//loop push end


    if (flaglist.empty()) zpipeIn.setTimeOut(1000); else zpipeIn.setTimeOut(0);
    flaglist.clear();

    return 0;
}