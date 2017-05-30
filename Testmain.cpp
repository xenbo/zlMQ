#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <pushout.h>
#include "getcmdFromzmq.h"

using namespace std;

#define clientNum 100


std::string topic1 = "topicxxxx";
std::string value1 = "ABCJHSJKFKLJKLHK";

getcmdFromzmq *docmd[4];

void *test(void *arg) {
    for (int j = 0; j < 50000; j++) {
        for (int i = 0; i < clientNum; i++) {
            char valuetmp[100];
            sprintf(valuetmp, "_%d_%d", i, j);
            char keytmp[100];
            sprintf(keytmp, "_%d", i);

            std::string key = topic1 + keytmp;
            std::string value = value1 + valuetmp;


            JOB *job = new JOB(0, 100 + i, 1011, key, (char *) value.c_str(), value.length());
            unsigned long addr = (unsigned long) job;
            docmd[i % 4]->zpipeIn.sneddata((char *) &addr, sizeof(unsigned long));
        }
    }
}

int main() {

    pushout *push = new pushout();
    getcmdFromzmq::getcmdFromzmqInit(4);
    for (int i = 0; i < 4; i++) {
        docmd[i] = getcmdFromzmq::SvrList[i];
        push->setzpipe(&docmd[i]->zpipeOut);
    }
    push->Start();

    pthread_t tid;
    pthread_create(&tid, NULL, test, NULL);

    sleep(10);
    for (int i = 0; i < clientNum; i++) {
        sleep(2);
        char keytmp[100];
        sprintf(keytmp, "_%d", i);
        std::string key = topic1 + keytmp;

        JOB *job1 = new JOB(1, 100 + i, 1011, key, NULL, 0);
        unsigned long addr1 = (unsigned long) job1;
        job1->consumline = -1;
        docmd[i % 4]->zpipeIn.sneddata((char *) &addr1, sizeof(unsigned long));
    }

    getchar();

    return 0;
}