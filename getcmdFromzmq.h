//
// Created by dongbo on 5/16/17.
//

#ifndef LEVELDBTEST_GETCMDFROMZMQ_H
#define LEVELDBTEST_GETCMDFROMZMQ_H
#include <iostream>
#include <map>
#include <vector>
#include <unistd.h>
#include "job.h"
#include "zmqpipe.h"
#include "opleveldb.h"

class getcmdFromzmq {
public:
    static void getcmdFromzmqInit(int num);
    ~getcmdFromzmq();
public:
    static std::vector<getcmdFromzmq*> SvrList;
    zmqpipe zpipeIn;
    zmqpipe zpipeOut;

private:
    static void *Run(void *arg);
    static std::vector<getcmdFromzmq*> createlist(){
        std::vector<getcmdFromzmq*> _SvrList;
        return _SvrList;
    }

    getcmdFromzmq(int id);
    int GetcmdtimeNOut(JOB* job);
    int GetcmdtimeOut();


private:
    std::map<std::string, std::map<int, opleveldb *> > doCmdFindDB;
    int id;
    static int numSvr;


    std::map<std::string, std::map<int, opleveldb *> > opPushlist1;
    std::map<std::string, std::map<int, opleveldb *> > opPushlist2;

    std::list<JOB*> pushlist;

    long num, numall;
    unsigned int t1, t2;
};



#endif //LEVELDBTEST_GETCMDFROMZMQ_H
