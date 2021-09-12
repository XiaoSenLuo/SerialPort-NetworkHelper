#include "tcpthread.h"

TCPThread::TCPThread(QObject *parent) : QObject(parent)
{

}

TCPThread::~TCPThread()
{
    if(tcpServer && tcpServer->isListening()) tcpServer->close();
    tcpServer->deleteLater();

    if(tcpSocket&& tcpSocket->isOpen()) tcpSocket->close();
    tcpSocket->deleteLater();
}
