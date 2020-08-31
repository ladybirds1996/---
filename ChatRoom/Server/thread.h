#ifndef THREAD_H
#define THREAD_H

#include <QObject>
#include <QThread>

// 封装的线程类
class Thread :public QObject
{
    Q_OBJECT
public:
    Thread();
    ~Thread();
    void Thread_Fun(void);
};

#endif // THREAD_H
