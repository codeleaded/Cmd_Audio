#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

typedef struct Thread {
    pthread_t h;
    void* attr;
    void* (*func)(void*);
    void* arg;
    char running;
} Thread;

typedef unsigned long long Duration;

#define MILLI_SECONDS   1000ULL
#define MICRO_SECONDS   1000000ULL
#define NANO_SECONDS    1000000000ULL

Thread Thread_New(void* attr,void* (*func)(void*),void* arg){
    Thread t;
    t.h = 0UL;
    t.attr = attr;
    t.func = func;
    t.arg = arg;
    t.running = 0;
    return t;
}
Thread Thread_Null(){
    Thread t;
    t.attr = NULL;
    t.func = NULL;
    t.arg = NULL;
    return t;
}
void Thread_Start(Thread* t){
    t->running = 1;
    if (pthread_create(&t->h,t->attr,t->func,t->arg) != 0) {
        perror("[Threads]: Start: Error");
        exit(1);
    }
}
void Thread_Join(Thread* t,void** ret){
    if(t->h) pthread_join(t->h,ret);
}
void Thread_Detach(Thread* t,void** ret){
    if(t->h) pthread_detach(t->h);
}
void Thread_Cancel(Thread* t,void** ret){
    if(t->h) pthread_cancel(t->h);
}
void Thread_Term(Thread* t,void** ret){
    if(t->h) pthread_kill(t->h,SIGTERM);
}
void Thread_Restart(Thread* t){
    if(t->running){
        if(t->h) Thread_Term(t,NULL);
        Thread_Start(t);
    }else{
        Thread_Start(t);
    }
}
void Thread_ChangeRestart(Thread* t,void* (*func)(void*)){
    t->func = func;
    if(t->running){
        if(t->h) Thread_Cancel(t,NULL);
        if(t->h) Thread_Join(t,NULL);
        Thread_Start(t);
    }else{
        Thread_Start(t);
    }
}
void Thread_Stop(Thread* t){
    t->running = 0;
    if(t->h) Thread_Cancel(t,NULL);
    if(t->h) Thread_Join(t,NULL);
}

void Thread_Sleep_S(Duration secs){
    struct timespec req = { secs,0 };
    nanosleep(&req, NULL);
}
void Thread_Sleep_M(Duration msecs){
    struct timespec req = { msecs / MILLI_SECONDS,msecs % MILLI_SECONDS + MICRO_SECONDS };
    nanosleep(&req, NULL);
}
void Thread_Sleep_U(Duration usecs){
    struct timespec req = { usecs / MICRO_SECONDS,usecs % MICRO_SECONDS + MILLI_SECONDS };
    nanosleep(&req, NULL);
}
void Thread_Sleep_N(Duration nsecs){
    struct timespec req = { nsecs / NANO_SECONDS,nsecs % NANO_SECONDS };
    nanosleep(&req, NULL);
}

#endif //!THREAD_H