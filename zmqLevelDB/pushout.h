//
// Created by dongbo01 on 5/23/17.
//

#ifndef MQSVR2_PUSHOUT_H
#define MQSVR2_PUSHOUT_H

#include <vector>
#include "zmqpipe.h"
#include "job.h"



#include "protocol/server/MQLBSvrXY.h"
#include "protocol/server/MQSvrXY.h"
#include "protocol/30/ManageXY.h"
#include "common/ProtocolHead.h"
#include "common/MultiServerSocket.h"
#include "MQtoMQLBSvr.h"


class pushout {

public:
    int setzpipe(zmqpipe *pipe);

    int Start();

    void setmqlb(MQtoMQLBSvr *mqlbSvr);

private:
    static void *Run(void *arg);

    void push();

    template<typename T>
    int sendMess(T msg, JOB* eTask);

    void FUNC_CMD_SET(JOB *job);

    void FUNC_CMD_GET(JOB *job);

    void FUNC_CMD_CLOSE(JOB *job);

    void FUNC_CMD_ACK(JOB *job);

    void FUNC_CMD_CLIENT_CLOSE_TOPIC(JOB *job);

    void FUNC_CMD_LB_CLOSE_TOPIC(JOB *job);

    void FUNC_CMD_ACK_PULL(JOB *job);


private:
    std::vector<zmqpipe *> zpipelist;
    MQtoMQLBSvr *mqlbSvr;
};

template<typename T>
int pushout::sendMess(T msg, JOB* eTask) {

    char mess[4096];
    OldClientHead *head = (OldClientHead *) mess;
    head->xyid = msg.XY_ID;

    bostream bos;
    bos.attach(mess + 4, 4000);
    bos << msg;
    head->len = bos.length();

    CIOSocket *Socket = static_cast<CIOSocket *>(eTask->t);
    assert(Socket != NULL);

    int f = Socket->AsynSend(mess, head->len + 4);

    if (!f)
        return head->len + 4;
    return -1;
}

#endif //MQSVR2_PUSHOUT_H
