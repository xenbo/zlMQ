//
// Created by dongbo on 5/16/17.
//
#include <cassert>
#include "common/Log.h"
#include "getcmdFromzmq.h"


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
    unsigned long long num = 0;

    prctl(PR_SET_NAME, "GETCMD_THREAD");

    getcmdFromzmq *me = (getcmdFromzmq *) arg;
    me->zpipeIn.setTimeOut(0);
    me->zpipeConsumer.setTimeOut(0);

    while (true) {
        unsigned long buf[1];
        int len = me->zpipeIn.getdata((char *) buf, 8);
        if (len < 0) {
            num++;
            me->GetcmdtimeOut();
        } else {
            JOB *eTask = (JOB *) buf[0];
            me->GetcmdtimeNOut(eTask);
            delete eTask;
            num = 0;
        }

        len = me->zpipeConsumer.getdata((char *) buf, 8);
        if (len > 0) {
            JOB *eTask = (JOB *) buf[0];
            me->GetcmdtimeNOut(eTask);
            delete eTask;
        }

        // std::cout << JOB::num << std::endl;
        if (num > 2000) me->zpipeIn.setTimeOut(5);
        if (num == 0) me->zpipeIn.setTimeOut(0);
    }


    return NULL;
}


int getcmdFromzmq::GetcmdtimeNOut(JOB *eTask) {

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

    return 0;
}

int getcmdFromzmq::GetcmdtimeOut() {

    for (std::map<std::string, std::map<int, opleveldb *> >::iterator iter = opPushlist1.begin();
         iter != opPushlist1.end(); iter++) {
        std::string data;
        std::string optopic = iter->first;
        std::map<int, opleveldb *> &Opmap = iter->second;

        for (std::map<int, opleveldb *>::iterator opiter = Opmap.begin(); opiter != Opmap.end(); opiter++) {
            int consumAppid = opiter->first;
            opleveldb *opdbloop = opiter->second;

            if (opdbloop) {

                long long line = -1;
                int getFlag = opdbloop->opget(optopic, &data, line);
                if (getFlag == 0) {
                    LOGI("get OK,topic:" << optopic << ",linenum:" << line);
                } else if (getFlag < 0) {
//                    LOGI("get NOOK,topic:" << optopic << ",linenum:" << line << ",flag=" << getFlag);
                }

                if (getFlag >= 0) {
                    JOB *job = new JOB(CMD_GET, consumAppid, 0, optopic, (char *) data.c_str(), data.length());
                    job->t = opdbloop->t;
                    job->consumline = line;

                    unsigned long addr = (unsigned long) job;
                    zpipeOut.sneddata((char *) &addr, sizeof(unsigned long));
                }
            }
        }
    }//loop push end


    return 0;
}


void getcmdFromzmq::FUNC_CMD_SET(JOB *eTask) {
    std::string topic = eTask->topic;
    opleveldb *opdb = doCmdFindDB[topic][eTask->groupid][eTask->appid];
    if (!opdb) {
        opdb = new opleveldb(eTask->appid, eTask->groupid);
        opdb->opinitdb(topic);
        doCmdFindDB[topic][eTask->groupid][eTask->appid] = opdb;
    }

    int setFlag = opdb->opput(topic, std::string(eTask->p, eTask->slen)); //setFlag == lineNUM
    if (setFlag >= 0) {
        if (eTask->Productp) sprintf(eTask->Productp, "%s:%ld\n", eTask->topic, setFlag);
        LOGI("into OK,topic:" << topic << ",t_Groupid:" << eTask->groupid << ",len:" << eTask->slen << ",appid="
                              << eTask->appid << ",offset:" << setFlag << ",mess:" << eTask->p);
    } else if (setFlag < 0) {
        LOGI("into NOOK,topic:" << topic << ",t_Groupid:" << eTask->groupid << ",len:" << eTask->slen << ",appid="
                                << eTask->appid << ",offset:" << setFlag);
    }

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
                long long line = -1;
                int getFlag = opdbloop->opget(optopic, &data, line);
                if (getFlag == 0) {
                    if (opdb->ClineAddr) sprintf(opdb->ClineAddr, "%s:%ld\n", topic.c_str(), line);
                    LOGI("get OK,topic:" << optopic << ",linenum:" << line);
                } else if (getFlag < 0) {
                    LOGI("get NOOK,topic:" << optopic << ",linenum:" << line << ",flag=" << getFlag);
                }

                if (getFlag >= 0) {
                    JOB *job = new JOB(CMD_GET, consumAppid, eTask->groupid, optopic, (char *) data.c_str(),
                                       data.length());
                    job->t = opdbloop->t;
                    job->consumline = line;

                    unsigned long addr = (unsigned long) job;
                    zpipeOut.sneddata((char *) &addr, sizeof(unsigned long));
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
}

void getcmdFromzmq::FUNC_CMD_GET(JOB *eTask) {
    std::string topic = eTask->topic;
    opleveldb *opdb = doCmdFindDB[topic][eTask->groupid][eTask->appid];
    if (!opdb) {
        opdb = new opleveldb(eTask->appid, eTask->groupid);
        opdb->opinitdb(topic);
        doCmdFindDB[topic][eTask->groupid][eTask->appid] = opdb;
    }
    opdb->t = eTask->t;

    long long line = opdb->setStarConsumerLine(topic, (long long) eTask->consumline);
    opdb->ClineAddr = eTask->Consump;
    LOGI("Reg consume:" << eTask->topic << ",Groupid:" << eTask->groupid << ",offset:" << line);

    if (eTask->pullflag) {
        std::string data;
        int getFlag = opdb->opget(topic, &data, line);
        if (getFlag == 0) {
            if (opdb->ClineAddr) sprintf(opdb->ClineAddr, "%s:%ld\n", topic.c_str(), line);
            LOGI("get OK,topic:" << topic << ",linenum:" << line);
        } else if (getFlag < 0) {
            LOGI("get NOOK,topic:" << topic << ",linenum:" << line << ",flag=" << getFlag);
        }

        JOB *job = NULL;
        if (getFlag >= 0) {
            job = new JOB(CMD_ACK_PULL, eTask->appid, eTask->groupid, topic, (char *) data.c_str(), data.length());
            job->consumline = line;
        } else {
            job = new JOB(CMD_ACK_PULL, eTask->appid, eTask->groupid, topic, NULL, 0);
            job->consumline = line - 1;
        }

        job->t = opdb->t;
        unsigned long addr = (unsigned long) job;
        zpipeOut.sneddata((char *) &addr, sizeof(unsigned long));
    } else
        opPushlist1[topic][eTask->appid] = opdb;
}

void getcmdFromzmq::FUNC_CMD_CLOSE(JOB *eTask) {
    std::string topic = eTask->topic;
    opleveldb *opdb = doCmdFindDB[topic][eTask->groupid][eTask->appid];
    if (!opdb) {
        opdb = new opleveldb(eTask->appid, eTask->groupid);
        opdb->opinitdb(topic);
        doCmdFindDB[topic][eTask->groupid][eTask->appid] = opdb;
    }

    if (!opPushlist1[eTask->topic].empty()) {
        std::map<int, opleveldb *> &tmpTab = opPushlist1[eTask->topic];
        std::map<int, opleveldb *>::iterator it = tmpTab.find(eTask->appid);
        if (it != tmpTab.end()) {
            tmpTab.erase(it);
            LOGI("### close, erase, topic:" << topic << ",appid:" << eTask->appid)
        }
    }
}

void getcmdFromzmq::FUNC_CMD_ACK(JOB *eTask) {
    std::string topic = eTask->topic;
    opleveldb *opdb = doCmdFindDB[topic][eTask->groupid][eTask->appid];
    if (!opdb) {
        opdb = new opleveldb(eTask->appid, eTask->groupid);
        opdb->opinitdb(topic);
        doCmdFindDB[topic][eTask->groupid][eTask->appid] = opdb;
    }

    char tmp[1024];
    sprintf(tmp, "%s-%d", topic.c_str(), eTask->appid);
    std::string keyuser = tmp;
    sprintf(tmp, "_%lld", eTask->consumline);
    std::string LineData = tmp;
    opdb->haveAck = true;
    int flag = opdb->opputline(topic, keyuser, LineData);    //consumLine write into file

    LOGI("ack, topic:" << eTask->topic << ",appid:" << eTask->appid << ",consumLine=" << eTask->consumline);
}

void getcmdFromzmq::FUNC_CMD_CLIENT_CLOSE_TOPIC(JOB *eTask) {
    std::string topic = eTask->topic;
    opleveldb *opdb = doCmdFindDB[topic][eTask->groupid][eTask->appid];
    if (!opdb) {
        opdb = new opleveldb(eTask->appid, eTask->groupid);
        opdb->opinitdb(topic);
        doCmdFindDB[topic][eTask->groupid][eTask->appid] = opdb;
    }
    if (!opPushlist1[eTask->topic].empty()) {
        std::map<int, opleveldb *> &tmpTab = opPushlist1[eTask->topic];
        std::map<int, opleveldb *>::iterator it = tmpTab.find(eTask->appid);
        if (it != tmpTab.end()) {
            tmpTab.erase(it);
            LOGI("### close, erase, topic:" << topic << ",appid:" << eTask->appid)
        }
    }


    std::ostringstream osstr;
    osstr << topic << "-" << eTask->appid;
    std::string data;
    opdb->opgetline(topic, osstr.str(), &data);

    JOB *job = new JOB(CMD_CLIENT_CLOSE_TOPIC, eTask->appid, eTask->groupid, topic, NULL, 0);
    job->LBAskid = eTask->LBAskid;
    job->flag = job->one;
    job->t = opdb->t;
    job->offset = atoi(data.c_str() + 1);
    unsigned long addr = (unsigned long) job;
    zpipeOut.sneddata((char *) &addr, sizeof(unsigned long));

    LOGI("GroupTopic:" << topic << ",offset:" << ",keyuser:" << osstr.str() << ",data:" << data << ",offset"
                       << job->offset);
}

void getcmdFromzmq::FUNC_CMD_LB_CLOSE_TOPIC(JOB *eTask) {
    std::string topic = eTask->topic;
    opleveldb *opdb = doCmdFindDB[topic][eTask->groupid][eTask->appid];
    if (!opdb) {
        opdb = new opleveldb(eTask->appid, eTask->groupid);
        opdb->opinitdb(topic);
        doCmdFindDB[topic][eTask->groupid][eTask->appid] = opdb;
    }

    std::ostringstream osstr;
    osstr << topic << "-" << eTask->appid;
    std::string data;
    opdb->opgetline(topic, osstr.str(), &data);

    JOB *job = new JOB(CMD_LB_CLOSE_TOPIC, eTask->appid, eTask->groupid, topic, NULL, 0);
    job->LBAskid = eTask->LBAskid;
    job->flag = job->one;
    job->t = opdb->t;
    job->offset = atoi(data.c_str() + 1);
    unsigned long addr = (unsigned long) job;
    zpipeOut.sneddata((char *) &addr, sizeof(unsigned long));
}

void getcmdFromzmq::FUNC_CMD_ACK_PULL(JOB *eTask) {
    std::string topic = eTask->topic;
    opleveldb *opdb = doCmdFindDB[topic][eTask->groupid][eTask->appid];
    if (!opdb) {
        opdb = new opleveldb(eTask->appid, eTask->groupid);
        opdb->opinitdb(topic);
        doCmdFindDB[topic][eTask->groupid][eTask->appid] = opdb;
    }

    std::ostringstream osstr;
    osstr << topic << "-" << eTask->appid;
    std::string keyuser = osstr.str();


    std::ostringstream osstrline;
    osstrline << "_" << eTask->consumline - 1;
    std::string data = osstrline.str();
    opdb->opputline(topic, keyuser, data);

    LOGI("ack pull :" << keyuser << ",line:" << data);

    data.clear();
    int getFlag = opdb->opget(topic, &data, eTask->consumline);
    if (getFlag == 0) {
        if (opdb->ClineAddr) sprintf(opdb->ClineAddr, "%s:%ld\n", topic.c_str(), eTask->consumline);
        LOGI("get OK,topic:" << topic << ",linenum:" << eTask->consumline);
    } else if (getFlag < 0) {
        LOGI("get NOOK,topic:" << topic << ",linenum:" << eTask->consumline << ",flag=" << getFlag);
    }

    JOB *job = NULL;
    if (getFlag >= 0) {
        job = new JOB(CMD_ACK_PULL, eTask->appid, eTask->groupid, topic, (char *) data.c_str(), data.length());
        job->consumline = eTask->consumline;
    } else {
        job = new JOB(CMD_ACK_PULL, eTask->appid, eTask->groupid, topic, NULL, 0);
        job->consumline = eTask->consumline - 1;
    }

    job->t = eTask->t;
    unsigned long addr = (unsigned long) job;
    zpipeOut.sneddata((char *) &addr, sizeof(unsigned long));
}
