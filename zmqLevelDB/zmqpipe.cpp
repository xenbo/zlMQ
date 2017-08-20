//
// Created by dongbo on 5/16/17.
//

#include "zmqpipe.h"
#include <cassert>

int zmqpipe::id = 0;

int zmqpipe::sneddata(char *content, int len) {
    int rc = 0;
    pthread_spin_lock(&lock);
    rc = zmq_send(start, content, len, 0);
    pthread_spin_unlock(&lock);
    return rc;
}

int zmqpipe::sneddata(unsigned long addr) {
    int rc = 0;
    pthread_spin_lock(&lock);
    addrlist.push_back(addr);
    pthread_spin_unlock(&lock);
    return rc;
}


int zmqpipe::setTimeOut(int time) {
    timeOut = time;

//    zmq_setsockopt(end, ZMQ_RCVTIMEO, &timeOut, sizeof(timeOut));
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
//        std::cout << timeOut << std::endl;
    } else {
        //std::cout << "timeout..............." << std::endl;
    }

    return rc;
}


unsigned long zmqpipe::getdata() {
    unsigned long rc = 0;
    pthread_spin_lock(&lock);
    if (!addrlist.empty()) {
        rc = addrlist.front();
        addrlist.pop_front();
    }
    pthread_spin_unlock(&lock);

    return rc;
}


zmqpipe::zmqpipe() {
    int num = 0;
    ctx = zmq_ctx_new();
    assert(ctx);

    end = zmq_socket(ctx, ZMQ_PAIR);
    assert(end);
    zmq_setsockopt(end, ZMQ_RCVHWM, (const void *) &num, sizeof(int));

    std::ostringstream osstr;
    osstr << "inproc://pipeline_xx_" << __sync_add_and_fetch(&id, 1);

    int rc = zmq_bind(end, osstr.str().c_str());
    assert(rc == 0);

    start = zmq_socket(ctx, ZMQ_PAIR);
    assert(start);

    zmq_setsockopt(start, ZMQ_SNDHWM, (const void *) &num, sizeof(int));

    rc = zmq_connect(start, osstr.str().c_str());
    assert(rc == 0);

//
//    timeOut = 10;
//    zmq_setsockopt(end, ZMQ_RCVTIMEO, &timeOut, sizeof(timeOut));


    timeOut = 0;
    pthread_spin_init(&lock, 0);
}

zmqpipe::~zmqpipe() {

    int rc = zmq_close(start);
    assert(rc == 0);

    rc = zmq_close(end);
    assert(rc == 0);

    rc = zmq_ctx_term(ctx);
    assert(rc == 0);
}