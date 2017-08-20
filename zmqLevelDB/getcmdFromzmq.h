//
// Created by dongbo on 5/16/17.
//

#ifndef LEVELDBTEST_GETCMDFROMZMQ_H
#define LEVELDBTEST_GETCMDFROMZMQ_H

#include <iostream>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/prctl.h>

#include "job.h"
#include "zmqpipe.h"
#include "opleveldb.h"


const int CMD_SET = 0;
const int CMD_GET = 1;
const int CMD_CLOSE = 4;
const int CMD_ACK = 8;
const int CMD_CLIENT_CLOSE_TOPIC = 9;
const int CMD_LB_CLOSE_TOPIC = 10;
const int CMD_ACK_PULL = 11;

class getcmdFromzmq {
public:
    static void getcmdFromzmqInit(int num);

    ~getcmdFromzmq();

public:
    static std::vector<getcmdFromzmq *> SvrList;
    zmqpipe zpipeIn;
    zmqpipe zpipeOut;
    zmqpipe zpipeConsumer;

private:
    static void *Run(void *arg);

    static std::vector<getcmdFromzmq *> createlist() {
        std::vector<getcmdFromzmq *> _SvrList;
        return _SvrList;
    }

    getcmdFromzmq(int id);

    int GetcmdtimeNOut(JOB *job);

    int GetcmdtimeOut();


    void FUNC_CMD_SET(JOB *job);

    void FUNC_CMD_GET(JOB *job);

    void FUNC_CMD_CLOSE(JOB *job);

    void FUNC_CMD_ACK(JOB *job);

    void FUNC_CMD_CLIENT_CLOSE_TOPIC (JOB * job);

    void FUNC_CMD_LB_CLOSE_TOPIC(JOB *job);

    void FUNC_CMD_ACK_PULL(JOB *job);


private:
    std::map<std::string, std::map<int,std::map<int, opleveldb *> > > doCmdFindDB;
    int id;
    static int numSvr;


    std::map<std::string, std::map<int, opleveldb *> > opPushlist1;
    std::map<std::string, std::map<int, opleveldb *> > opPushlist2;

    std::list<JOB *> pushlist;

    long num, numall;
    unsigned int t1, t2;
};


#endif //LEVELDBTEST_GETCMDFROMZMQ_H
