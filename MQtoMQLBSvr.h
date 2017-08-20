//
// Created by dongbo01 on 5/16/17.
//

#ifndef MQSVR2_MQTOMQLBSVR_H
#define MQSVR2_MQTOMQLBSVR_H

#include <iostream>
#include <map>
#include <set>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/thread.h>
#include <event2/util.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/mman.h>


#include "statgrab.h"
#include "define.h"

#include "common/MultiServerSocket.h"
#include "common/biostream.h"
#include "common/Log.h"
#include "common/IOSocket.h"
#include "protocol/server/MQSvrXY.h"
#include "protocol/server/MQLBSvrXY.h"
#include "common/ProtocolHead.h"

#define MAXDOCMD 10
#define CONSUMER_TOPIC_LINE  "./LINE/CONSUME"
#define PRODUCTER_TOPIC_LINE  "./LINE/PRODUCT"
#define MaxTopicNum 20000

struct MQLBSvrInfo {
    std::string ip;
    int port;
};


struct Consuminfo {
    int groupid;
    int nAppid;
    int mq_appid;
    CIOSocket *soket;
    std::string TopicName;
    char *lineAddr;

    Consuminfo(std::string TopicName, int _groupid, int _nAppid, CIOSocket *_soket, int mqAppid) {
        groupid = _groupid;
        nAppid = _nAppid;
        mq_appid = mqAppid;
        soket = _soket;
        TopicName = TopicName;

        char cmd[1024] = {0};
        sprintf(cmd, "mkdir -p %s/%d", CONSUMER_TOPIC_LINE, mqAppid);
        system(cmd);

        //mmap
        char file[1024];
        sprintf(file, "%s/%d/%s_%d-%d", CONSUMER_TOPIC_LINE, mqAppid, TopicName.c_str(), groupid, _nAppid);
        int fd = open(file, O_RDWR | O_CREAT, 00777);
        lseek(fd, 101, SEEK_SET);
        write(fd, "", 1);
        lineAddr = (char *) mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        memset(lineAddr + 99, 0, 1);
        close(fd);
    }
};

struct topicInfo {
    int groupid;
    int Maxlen;
    std::string TopicName;
    char *lineAddr;
    long long StartDISKLineNum;
    long long EndDISKLineNum;
    std::map<int, Consuminfo *> appid2Consum; //appid -> consumlineNum

    void createMMAP(int mqAppid) {
        //mmap
        char cmd[1024] = {0};
        sprintf(cmd, "mkdir -p %s/%d", PRODUCTER_TOPIC_LINE, mqAppid);
        system(cmd);

        char file[1024];
        sprintf(file, "%s/%d/%s_%d", PRODUCTER_TOPIC_LINE, mqAppid, TopicName.c_str(), groupid);
        int fd = open(file, O_RDWR | O_CREAT, 00777);
        lseek(fd, 101, SEEK_SET);
        write(fd, "", 1);
        lineAddr = (char *) mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        memset(lineAddr + 99, 0, 1);
        close(fd);
    }

};

struct staticRec {
    char m_pRecvBuf[4096];
    int m_nRecvBufUsed;
    int m_nBufSize;

    staticRec() {
        m_nRecvBufUsed = 0;
        m_nBufSize = 4096;
    }
};


class MQtoMQLBSvr {

public:
    int start();

    MQtoMQLBSvr(int _appid, int _groupid);

    int SetMQLbSvripport(std::string ip_, int port_);

    int getTopicId(std::string topic, int groupid);

    void SettopicList(std::set<std::string> &topicList);

    char *GetTopicAddr(int id);

    ~MQtoMQLBSvr();

    void setServerSocket(CMultiServerSocket *_m_pServerSocket);

    template<typename T>
    int putMessToLB(T mess);

private:
    static void OnTimer(int fd, short event, void *arg);

    void topicListReadFromdisk();

    int Connect(char *pRemoteIp, unsigned short nPort);

    int Connect();

    int sendReg();

    int MQLBSvrRegRsp(char *buf, int len);

    int sendBalance();

    int setTopicId(std::string topic, int groupid);

    int RecvCreateTopic(char *buf, int len);

    int RecTopicListFromMQLBSvr(char *buf, int len);

    int rspCreateTopic(int nAskID, int flag);

    int reqTopicList();

    int rspTopicList();

    int MasterCreateTopic(char szTopic[256], int nContentSize, int nGroupID);

    static void ReadEvent(bufferevent *bev, void *arg);

    static void CloseEvent(struct bufferevent *bev, short events, void *user_data);

    static void *Run(void *arg);

    void evntloop();

    int FliterStream(staticRec *s, char *buf, int size);

    int doRec(char *buf, int len);

    int LBMQCloseClient(char *buf, int len);

    int SendMess(char *buf, int len);


private:

    int cmdfd[2];
    int appid;
    int groupid;
    struct event_base *eventBase;
    bufferevent *bev;
    bufferevent *bevpipe;
    std::vector<MQLBSvrInfo> mqlblist;
    topicInfo TopicList[MaxTopicNum];
    std::map<int, std::map<std::string, int> > topicMapId;
    pthread_spinlock_t topicidlock;
    pthread_spinlock_t topicListlock;
    pthread_mutex_t lock;


    int numTopic;
    CMultiServerSocket *m_pServerSocket;


    event *TimerEvent;
};

template<typename T>
int MQtoMQLBSvr::putMessToLB(T mess) {
    OldClientHead head;
    memset(&head, 0, sizeof(head));
    head.xyid = mess.XY_ID;

    bostream bos;
    const int cbsize = sizeof(mess) + OLDCLIENTHEADLEN;
    char pdata[cbsize];

    bos.attach(pdata + OLDCLIENTHEADLEN, cbsize - OLDCLIENTHEADLEN);
    bos << mess;
    head.len = bos.length();
    memcpy(pdata, &head, OLDCLIENTHEADLEN);

    int f = SendMess(pdata, head.len + 4);
    return f;
}


#endif //MQSVR2_MQTOMQLBSVR_H
