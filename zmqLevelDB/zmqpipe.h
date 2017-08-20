//
// Created by dongbo on 5/16/17.
//

#ifndef LEVELDBTEST_ZMQPIPE_H
#define LEVELDBTEST_ZMQPIPE_H

#include <iostream>
#include <string>
#include <sstream>
#include <list>

#include "zmq.h"

class zmqpipe {
    static int id;

private:
    void *ctx;
    void *end;
    void *start;
    int timeOut;
    std::list<unsigned long> addrlist;
public:
    int sneddata(unsigned long addr);

    int sneddata(char *content, int len);

    int getdata(char *buffer, int len);

    unsigned long getdata();

    int setTimeOut(int time);

    zmqpipe();

    ~zmqpipe();

    void *getPipEnd() {
        return end;
    }

    pthread_spinlock_t lock;

};


#endif //LEVELDBTEST_ZMQPIPE_H
