#include "thread.h"
#include <QDebug>

Thread::Thread() {
    qDebug()<<"Thread构造函数ID:"<<QThread::currentThreadId();
}

Thread::~Thread() {
    // 析构，摧毁线程
}

void Thread::Thread_Fun() {
    qDebug()<<"子线程功能函数ID:"<<QThread::currentThreadId();
}
