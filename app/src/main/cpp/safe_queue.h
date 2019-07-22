//
// Created by guo on 2019-06-17.
//
#ifndef RTMPPUSH_SAGEQUEUE_H
#define RTMPPUSH_SAGEQUEUE_H
#include <pthread.h>
#include <queue>

using namespace std;


template<typename T>
class SafeQueue {
private:
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    qureue<T> q;
    //是否工作的标记 1：工作 0：不接受数据 不工作
    int work;
protected:

public:
    SafeQueue() {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
    }

    ~SafeQueue() {
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
    }

    void put(T value) {
        pthread_mutex_lock(&mutex);
        if (work) {
            q.push(value);
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutex);
    }

    int get(T &value) {
        int ret = 0;
        pthread_mutex_lock(&mutex);
        while (work && q.empty()) {
            pthread_cond_wait(&cond, &mutex);
        }
        if (!q.empty()) {
            value = q.front();
            q.pop();
            ret = 1;
        }
        pthread_mutex_unlock(&mutex);
        return ret;

    }

    void setWork(int work) {
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    int empty() {
        return q.empty();
    }

    int size() {
        return q.size();
    }

    void clear() {
        pthread_mutex_lock(&mutex);
        while (q.front()) {
            q.pop();
        }
        pthread_mutex_unlock(&mutex);

    }

    void sync() {
        pthread_mutex_lock(&mutex);
        pthread_mutex_unlock(&mutex);
    }

};

#endif //RTMPPUSH_SAGEQUEUE_H
