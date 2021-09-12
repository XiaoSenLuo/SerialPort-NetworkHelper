#ifndef UDPTHREAD_H
#define UDPTHREAD_H

#include <QObject>

class UDPThread : public QObject
{
    Q_OBJECT
public:
    explicit UDPThread(QObject *parent = nullptr);

signals:

};

#endif // UDPTHREAD_H
