//
// Created by dongbo01 on 5/11/17.
//

#include <dirent.h>
#include <cstdlib>

#include <cassert>

#include "opleveldb.h"

pthread_spinlock_t LDB::DBlock = {0};
pthread_once_t LDB::one_spin = PTHREAD_ONCE_INIT;
std::map<std::string, LDB *> LDB::TopicFindDB = LDB::createMap();

int LDB::appid = 0;

LDB::LDB(std::string Topic) {
    pthread_spin_lock(&DBlock);
    num = 0;

    woptions.sync = false;
    options.create_if_missing = true;
    options.block_size = 100 * 1024 * 1024;

    char tmpPAth[1024];
    sprintf(tmpPAth, "%s/%d/%s", DBPATH, appid, Topic.c_str());
    std::string path = tmpPAth;

    char cmd[1024];
    sprintf(cmd, "mkdir -p %s", tmpPAth);
    system(cmd);

    leveldb::Status status = leveldb::DB::Open(options, path, &db);

    if (!status.ok()) {
        std::cout << "topic:" << path << std::endl;
    }

    assert(status.ok());

    startLine = endLine = 0;
    TopicFindDB[Topic] = this;
    pthread_spin_unlock(&DBlock);
}

int LDB::Init(std::set<std::string> &topiclist) {
    DIR *dir;
    struct dirent *ptr;

    char tmpPAth[1024];
    sprintf(tmpPAth, "%s/%d/", DBPATH, appid);
    char cmd[1024];
    sprintf(cmd, "mkdir -p %s", tmpPAth);
    system(cmd);

    if ((dir = opendir(tmpPAth)) == NULL) {
        printf("Open dir error: %s\n", tmpPAth);
        return -1;
    }

    while ((ptr = readdir(dir)) != NULL) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        if (ptr->d_type == 4) {
            std::string topicName = std::string(ptr->d_name);
            topiclist.insert(topicName);
        }
    }
    for (std::set<std::string>::iterator it = topiclist.begin(); it != topiclist.end(); it++) {
        std::string topicName = *it;
        std::string data;
        LDB *_ldb = new LDB(topicName);
        _ldb->_get(topicName, &data);

        if (!data.empty()) {
            _ldb->startLine = atoi(data.c_str() + 1);
            _ldb->endLine = atoi(data.c_str() + data.find(":") + 1);
        }

        for (long long i = _ldb->startLine; i <= _ldb->endLine; i++) {
            char tmp[256];
            sprintf(tmp, "%s_%lld", topicName.c_str(), i);
            std::string key = tmp;
//            std::cout << "have key:" << key << std::endl;
            _ldb->dataList.push_back(key);
            _ldb->num++;
        }

        std::cout << topicName << ",start:" << _ldb->startLine << ",end:" << _ldb->endLine << std::endl;
        LDB::TopicFindDB[topicName] = _ldb;
    }


    return 0;
}

int LDB::_put(std::string key, std::string data) {
    leveldb::Status a = db->Put(leveldb::WriteOptions(), key, data);
    if (a.ok()) return 0;

//    DATATEST[key] =data;
    return -1;
}

int LDB::_get(std::string key, std::string *data) {
    leveldb::Status a = db->Get(leveldb::ReadOptions(), key, data);
    if (a.ok()) return 0;
    if (a.IsNotFound()) return -2;

//    *data = DATATEST[key];

    return -1;
}

int LDB::_delete(std::string key) {
    leveldb::Status a = db->Delete(leveldb::WriteOptions(), key);
    if (a.ok()) return 0;
    if (a.IsNotFound()) return -2;

//    std::map<std::string ,std::string>::iterator iter = DATATEST.find(key);
//    if(iter != DATATEST.end())
//        DATATEST.erase(iter);

    return 0;
}

opleveldb::opleveldb(int _appid, int _gropuid) {
    appid = _appid;
    groupid = _gropuid;
    haveAck = false;
}


LDB *opleveldb::opinitdb(std::string Topic, long long consustartLine) {
    LDB *ldb = NULL;
    ldb = LDB::getDB(Topic);
    if (ldb == NULL) ldb = new LDB(Topic);

    TopicFindDB[Topic] = ldb;

    char tmp[256];
    sprintf(tmp, "%s-%d", Topic.c_str(), appid);
    std::string keyuser = tmp;
    std::string LineData;
    ldb->_put(keyuser, LineData);

    long long &consumLine = consumMapLine[Topic];
    __sync_lock_test_and_set(&consumLine, consustartLine);

    return ldb;
}

LDB *opleveldb::opinitdb(std::string Topic) {
    LDB *ldb = NULL;
    ldb = LDB::getDB(Topic);
    if (ldb == NULL) ldb = new LDB(Topic);

    TopicFindDB[Topic] = ldb;

    char tmp[256];
    sprintf(tmp, "%s-%d", Topic.c_str(), appid);
    std::string keyuser = tmp;
    std::string LineData;
    ldb->_get(keyuser, &LineData);

    long long &consumLine = consumMapLine[Topic];
    __sync_lock_test_and_set(&consumLine, atoi(LineData.c_str() + 1));

    return ldb;
}

int opleveldb::opput(std::string topic, std::string data) {
    LDB *ldb = TopicFindDB[topic];
    if (ldb == NULL) {
        ldb = opinitdb(topic);
    }

    long long sline = __sync_fetch_and_add(&ldb->startLine, 0);
    long long eline = __sync_fetch_and_add(&ldb->endLine, 1);

    char tmp[256];
    sprintf(tmp, "%s_%010lld", topic.c_str(), eline);
    std::string key = tmp;
    int flag = -1;

    ldb->_put(key, data);
   // ldb->dataList.push_back(key);
    ldb->num++;

    if (ldb->num > MAXLENGTH) { //ldb->dataList.size()
        sline = __sync_fetch_and_add(&ldb->startLine, 1);

        sprintf(tmp, "%s_%010lld", topic.c_str(), sline);
        std::string key2 = tmp;
        ldb->_delete(key2);
        //ldb->dataList.pop_front();
        ldb->num--;
        std::cout << "Delete:" << key2 << ",size:" << ldb->dataList.size() << ":" << MAXLENGTH << std::endl;
    }

    sprintf(tmp, "_%lld:%lld", sline, eline);
    std::string LineData = tmp;
    flag = ldb->_put(topic, LineData);    //ldb->endLine ldb->statrLine into file

    if (flag == 0) return eline;

    return flag;
}


int opleveldb::opputline(std::string topic, std::string key, std::string data) {
    LDB *ldb = TopicFindDB[topic];
    if (ldb == NULL) {
        ldb = opinitdb(topic);
    }
    std::string &LineData = data;
    int flag = ldb->_put(key, LineData);    //ldb->endLine ldb->statrLine into file

    return flag;
}

int opleveldb::opgetline(std::string topic, std::string key, std::string *data) {
    LDB *ldb = TopicFindDB[topic];
    if (ldb == NULL) {
        ldb = opinitdb(topic);
    }

    int flag = ldb->_get(key, data);
    return flag;
}


int opleveldb::opget(std::string topic, std::string *data, long long &lineback) {
    LDB *ldb = TopicFindDB[topic];
    if (ldb == NULL) {
        ldb = opinitdb(topic);
    }

    long long &consumLine = consumMapLine[topic];
    long long line = __sync_fetch_and_add(&consumLine, 0);
    if (line >= __sync_fetch_and_add(&ldb->endLine, 0)) {
        return -1;
    }
    __sync_fetch_and_add(&consumLine, 1);

    long long datastartLine = __sync_fetch_and_add(&ldb->startLine, 0);
    if (line < datastartLine) {
        __sync_lock_test_and_set(&consumLine, datastartLine + 1);
        line = datastartLine;
    }

    if (lineback > 0)
        line = lineback;

    char tmp[256];
    sprintf(tmp, "%s_%010lld", topic.c_str(), line);
//    sprintf(tmp, "%s_%lld", topic.c_str(), line);
    std::string key = tmp;
    int flag = ldb->_get(key, data);
//
//    if (!haveAck) {
//        sprintf(tmp, "%s-%d", topic.c_str(), appid);
//        std::string keyuser = tmp;
//        sprintf(tmp, "_%lld", line);
//        std::string LineData = tmp;
//        ldb->_put(keyuser, LineData);    //consumLine write into file
//    }

    if (!flag) {
        lineback = line;
    }

    return flag;
}

int opleveldb::setStarConsumerLine(std::string Topic, long long consustartLine) {
    LDB *ldb = TopicFindDB[Topic];
    if (ldb == NULL) {
        ldb = opinitdb(Topic);
    }

    char tmp[256];
    sprintf(tmp, "%s-%d", Topic.c_str(), appid);
    std::string keyuser = tmp;
    long long &consumLine = consumMapLine[Topic];

    std::string LineData;
    if (consustartLine >= 0) {
        consumLine = consustartLine;
        char tmp[256] = {0};
        sprintf(tmp, "_%lld", consumLine);
        LineData = tmp;
        opputline(Topic, keyuser, LineData);
        __sync_lock_test_and_set(&consumLine, consustartLine);
    } else if (consustartLine == -1) {
        opgetline(Topic, keyuser, &LineData);
        __sync_lock_test_and_set(&consumLine, atoi(LineData.c_str() + 1));
    } else if (consustartLine == -2) {
        __sync_lock_test_and_set(&consumLine, ldb->endLine);
    }

    std::cout << "set cline Get:" << keyuser << ",consumerlineFlag=" << consustartLine << ",cline:" << LineData
              << std::endl;

    return consumLine;
}
