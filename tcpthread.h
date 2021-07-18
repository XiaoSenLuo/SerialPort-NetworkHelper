#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QTcpServer>
#include <QAbstractSocket>
#include <QTcpSocket>
#include <QtNetwork>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QDebug>
#include <QDataStream>

#include "settingconfig.h"

class TCPThread : public QObject
{
    Q_OBJECT
public:
    explicit TCPThread(QObject *parent = nullptr);
    ~TCPThread();
signals:

    void tcp_server_occur_error(int err);
    void tcp_server_new_connection(const QString& sourceip, int id);
    void tcp_server_started(const QString& serverip, int port);
    void tcp_server_client_disconnected(const QString& clientip, int id);
    void tcp_server_recieved_data(const QByteArray &data, int len, const QString& sourceip, int id);

    void tcp_client_recieved_data(const QByteArray &data, int len, const QString& sourceip, int id);
    void tcp_client_occur_error(int err);
    void tcp_client_connected(void);
    void tcp_client_disconnected(void);

public slots:

    void tcp_start(const QString& ip, int port, int role){
        if(role == NetWorkSettingConfig::CLIENT){
            tcp_client_connect(ip, port);
        }
        if(role == NetWorkSettingConfig::SERVER){
            tcp_server_listen_client(ip, port);
        }
    }

    void tcp_stop(int role){
        if(role == NetWorkSettingConfig::CLIENT){
            tcp_client_disconnect();
        }
        if(role == NetWorkSettingConfig::SERVER){
            tcp_server_stop();
        }
    }

    void tcp_send(const QByteArray& data, int len, int role, const QString& desip, int id = -1){
        if(role == NetWorkSettingConfig::CLIENT){
            if(!tcpSocket->isOpen()) return;
            tcp_client_send_data(data, len, desip, id);
        }
        if(role == NetWorkSettingConfig::SERVER){
            if(!tcpServer) return;
            if(!tcpServer->isListening()) return;
            if(tcpSocketSet.isEmpty()) return;

            tcp_server_send_data(data, len, desip, id);
        }
    }

private:
    bool tcp_server_close_connection(QTcpSocket* socket){
        disconnect(socket, &QTcpSocket::disconnected, this, &TCPThread::tcp_server_handle_client_disconnected);
//        disconnect(socket, &QTcpSocket::stateChanged, this, &TCPThread::tcp_handle_scoket_state_changed);
        disconnect(socket, &QTcpSocket::readyRead, this, &TCPThread::tcp_server_handle_ready_read);

        bool res = tcpSocketSet.removeOne(socket);

        const QHostAddress addr = QHostAddress(socket->peerAddress().toIPv4Address());
        emit tcp_server_client_disconnected(addr.toString(), socket->peerPort());
        if(socket->isOpen()) socket->close();
        socket->deleteLater();
        return res;
    }
    void tcp_server_close_all_connection(void){
        if(tcpSocketSet.isEmpty()) return;
        int ls = tcpSocketSet.size();
        QTcpSocket *socket;
        for(int i = 0; i < ls; i++){
            socket = tcpSocketSet.first();
            tcp_server_close_connection(socket);
        }
    }

    void tcp_server_listen_client(const QString &ip, int port){

        if(tcpServer == nullptr){
            tcp_server_create();
        }
        if(tcpServer->isListening()){
            tcp_server_close_all_connection();
            tcpServer->close();
        }

        if(tcpServer->listen(QHostAddress(ip), port) == false){
            emit tcp_server_occur_error(QHostAddress(ip).toIPv4Address());
            tcpServer->close();
            return;
        }else{
            emit tcp_server_started(ip, port);
        }
    }

    void tcp_server_stop(void){
        if(tcpServer == nullptr) return;
        if(tcpServer->isListening()){
            tcpServer->close();
            tcpServer->deleteLater();
            tcpServer = nullptr;
        }
    }


    void tcp_client_connect(const QString &ip, int port){
        if(!tcpSocket){
            tcp_client_create();
        }
        tcpSocket->abort();
        tcpSocket->connectToHost(ip, port);
    }

    void tcp_client_disconnect(void){
        if(!tcpSocket) return;
        tcpSocket->close();
        tcpSocket->deleteLater();
        tcpSocket = nullptr;
    }

    void tcp_client_handle_connected(void){
        emit tcp_client_connected();
    }

    void tcp_client_handle_disconnected(void){
        emit tcp_client_disconnected();
    }

    void tcp_client_handle_ready_read(void){
        int len = 0, tmp = tcpSocket->bytesAvailable();
        QByteArray in;
        while(tmp){
            in.append(tcpSocket->readAll());
            len += tmp;
            tmp = tcpSocket->bytesAvailable();
        }
        if(len){
            QHostAddress addr = QHostAddress(tcpSocket->peerAddress().toIPv4Address());

            emit tcp_client_recieved_data(in, len, addr.toString(), tcpSocket->peerPort());
        }
    }

    void tcp_client_send_data(const QByteArray& data, int len, const QString& desip, int id = -1){
        if(!tcpSocket) return;
         QHostAddress addr = QHostAddress(tcpSocket->peerAddress().toIPv4Address());
        if((desip == addr.toString()) && (id == tcpSocket->peerPort())){
            tcpSocket->write(data);
        }
    }

//    void tcp_handle_scoket_state_changed(QAbstractSocket::SocketState socketState){
//        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());

//        qDebug() << socketState;

//        if(socketState == QAbstractSocket::ConnectedState){
//            qDebug() << "new connection:" << socket->peerAddress() << ":" << socket->peerName() << ":" << socket->peerPort();
//        }
//        if(socketState == QAbstractSocket::UnconnectedState){
//            disconnect(socket, &QTcpSocket::stateChanged, this, &TCPThread::tcp_handle_scoket_state_changed);
//        }
//    }

    void tcp_server_handle_client_disconnected(void){
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());

        bool res = tcp_server_close_connection(socket);

        qDebug() << "tcp_server_handle_client_disconnected:disconnect[" << res << "]: " << socket->peerAddress().toString();
    }

    void tcp_server_handle_ready_read(){
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
        int len = 0, tmp = socket->bytesAvailable();
        QByteArray in;
        while(tmp){
            in.append(socket->readAll());
            len += tmp;
            tmp = socket->bytesAvailable();
        }
        emit tcp_server_recieved_data(in, len, QHostAddress(socket->peerAddress().toIPv4Address()).toString(), socket->peerPort());
    }

    void tcp_server_send_data(const QByteArray& data, int len, const QString& desip, int id){
        if(!tcpServer) return;
        if(!tcpServer->isListening()) return;
        if(tcpSocketSet.isEmpty()) return;
        QHostAddress addr;
        int ls = tcpSocketSet.size();
        QTcpSocket *socket = nullptr;
        int find = -1;
        for(int i = 0; i < ls; i++){
            socket = tcpSocketSet.at(i);
            addr = QHostAddress(socket->peerAddress().toIPv4Address());
            if(desip == addr.toString() && (id ==socket->peerPort())){
                find = i;
                break;
            }
        }
        if(find != -1){
            socket->write(data);
        }
    }

    void tcp_server_handle_new_connection(void){
        QTcpSocket* socket = nullptr;
        while(this-tcpServer->hasPendingConnections()){
            socket = this->tcpServer->nextPendingConnection();
            if(socket == nullptr) break;
            tcpSocketSet.append(socket);

            connect(socket, &QTcpSocket::disconnected, this, &TCPThread::tcp_server_handle_client_disconnected);
            connect(socket, &QTcpSocket::readyRead, this, &TCPThread::tcp_server_handle_ready_read);

//            qDebug() << "new connection:" << socket->peerAddress() << ":" << socket->peerName() << ":" << socket->peerPort();
            emit tcp_server_new_connection(QHostAddress(socket->peerAddress().toIPv4Address()).toString(), socket->peerPort());
        }
    }


private:

    QTcpServer *tcpServer = nullptr;
    QVector<QTcpSocket*> tcpSocketSet;

    QTcpSocket *tcpSocket = nullptr;

    void initConnect(void);

    void tcp_server_create(void){
        if(tcpServer) return;

        tcpServer = new QTcpServer(this);

        connect(tcpServer, &QTcpServer::acceptError, [=](QAbstractSocket::SocketError socketError){
            QTcpSocket * socket = qobject_cast<QTcpSocket*>(sender());
            int err = (int)socketError;
            qDebug() << "TCP Server Error:" << socketError;

            emit tcp_server_occur_error(err);
        });
        connect(tcpServer, &QTcpServer::newConnection, this, &TCPThread::tcp_server_handle_new_connection);
    }

    void tcp_client_create(void){
        if(tcpSocket) return;
        tcpSocket = new QTcpSocket(this);

        connect(tcpSocket, &QIODevice::readyRead, this, &TCPThread::tcp_client_handle_ready_read);

        connect(tcpSocket, &QTcpSocket::errorOccurred, [=](QAbstractSocket::SocketError socketError){
            QTcpSocket * socket = qobject_cast<QTcpSocket*>(sender());
            int err = (int)socketError;
            qDebug() << socketError;
//            if(socket->isOpen()) socket->close();
            emit tcp_client_occur_error(err);
        });

        connect(tcpSocket, &QTcpSocket::connected, this, &TCPThread::tcp_client_handle_connected);
        connect(tcpSocket, &QTcpSocket::disconnected, this, &TCPThread::tcp_client_handle_disconnected);
//        connect(tcpSocket, &QTcpSocket::stateChanged, this, &TCPThread::tcp_handle_scoket_state_changed);
    }

};

#endif // TCPTHREAD_H
