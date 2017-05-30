//
// Created by dongbo on 5/16/17.
//

#include "zmqpipe.h"
#include <cassert>

int zmqpipe::sneddata(char *content, int len) {
    int rc;
    rc = zmq_send(start, content, len, 0);
    return rc;

}

int zmqpipe::setTimeOut(int time) {
    timeOut = time;
    return time;
}

int zmqpipe::getdata(char *buffer, int len) {
    zmq_pollitem_t items[1];
    items[0].socket = end;
    items[0].events = ZMQ_POLLIN;

    int rc = -1;
    int num = zmq_poll(items, 1, timeOut);
    if (num) {
        rc = zmq_recv(end, buffer, len, 0);
        //std::cout << buffer << std::endl;
    } else {
        //std::cout << "timeout..............." << std::endl;
    }
    return rc;
}


zmqpipe::zmqpipe() {
    ctx = zmq_ctx_new();
    assert(ctx);

    end = zmq_socket(ctx, ZMQ_PAIR);
    assert(end);
    int rc = zmq_bind(end, "inproc://p1234");
    assert(rc == 0);

    start = zmq_socket(ctx, ZMQ_PAIR);
    assert(start);
    int num = 200000;
    zmq_setsockopt(start, ZMQ_SNDHWM, (const void *) &num, sizeof(int));

    rc = zmq_connect(start, "inproc://p1234");
    assert(rc == 0);

    timeOut = 0;
}

zmqpipe::~zmqpipe() {

    int rc = zmq_close(start);
    assert(rc == 0);

    rc = zmq_close(end);
    assert(rc == 0);

    rc = zmq_ctx_term(ctx);
    assert(rc == 0);
}