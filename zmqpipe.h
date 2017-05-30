//
// Created by dongbo on 5/16/17.
//

#ifndef LEVELDBTEST_ZMQPIPE_H
#define LEVELDBTEST_ZMQPIPE_H

#include <iostream>
#include "zmq.h"

class zmqpipe {
private:
    void *ctx;
    void *end;
    void *start;
    int timeOut;
public:
    int sneddata(char *content, int len);
    int getdata(char *buffer, int len);
    int setTimeOut(int time);
    zmqpipe();
    ~zmqpipe();


};




#endif //LEVELDBTEST_ZMQPIPE_H
