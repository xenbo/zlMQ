//
// Created by dongbo01 on 5/23/17.
//

#ifndef MQSVR2_PUSHOUT_H
#define MQSVR2_PUSHOUT_H

#include <vector>
#include "zmqpipe.h"
#include "job.h"


//#include "protocol/server/MQLBSvrXY.h"
//#include "protocol/server/MQSvrXY.h"
//#include "protocol/30/ManageXY.h"
//#include "common/ProtocolHead.h"

class pushout {

public:
    int setzpipe(zmqpipe *pipe);

    int Start();


private:
    static void *Run(void *arg);

    void push();

private:
    std::vector<zmqpipe *> zpipelist;

};


#endif //MQSVR2_PUSHOUT_H
