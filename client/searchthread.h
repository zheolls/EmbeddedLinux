#ifndef SEATHREAD_H
#define SEATHREAD_H
#include<qthread.h>
#include<qdialog.h>

class SeaThread : public QObject, public QThread{
    Q_OBJECT
public:
    QMutex pause;
public:
    SeaThread();
    ~SeaThread();
    virtual void run();
signals:
    void connectToServer();
};

#endif
