//
// Created by dongbo01 on 5/12/17.
//

#ifndef LEVELDBTEST_JOB_H
#define LEVELDBTEST_JOB_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

const int JOBMAXNUM = 2000000;
struct JOB {

    enum {
        one,
        all,
        NOOK,
    };


    int fd;
    int groupid;
    int topicid;
    int *isuse;
    long long offset;//真实offset
    int appid; //MM块中第几行
    int slen;
    char *Productp; //文件位置
    char *Consump; //文件位置
    long long consumline; //费者消费位置

    long long *pDStarline; //DISK 生产起始位置
    long long *pDEendline; //DISK 生产末尾位置
    char topic[256];
    char p[2048];
    void *t;
    int LBAskid;
    int flag;
    int pullflag;


    static long num;

    int xyid;

    JOB();

    JOB(void *t);

    JOB(JOB *job);

    JOB(int xyid, int _appid, int _groupid, std::string _topic, char *data, int len);



    ~JOB();
};


#endif //LEVELDBTEST_JOB_H
