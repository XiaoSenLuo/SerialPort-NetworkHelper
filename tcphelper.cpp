#include "tcphelper.h"

#include <QDebug>

TCPHelper::TCPHelper(int role, QObject *parent) : QObject(parent)
{
    if(role == Server){
        this->role = role;
        server = new QTcpServer(this);

        QObject::connect(server, &QTcpServer::newConnection, this, &TCPHelper::handle_tcp_server_new_connection);
        QObject::connect(server, &QTcpServer::acceptError, this, &TCPHelper::handle_tcp_server_error);
    }
    if(role == Client){
        this->role = role;
        QTcpSocket *socket = new QTcpSocket(this);
        sockets.append(socket);

        QObject::connect(socket, &QTcpSocket::connected, this, &TCPHelper::handle_tcp_socket_connected);
        QObject::connect(socket, &QTcpSocket::disconnected, this, &TCPHelper::handle_tcp_socket_disconnected);
        QObject::connect(socket, &QTcpSocket::errorOccurred, this, &TCPHelper::handle_tcp_socket_error);
        QObject::connect(socket, &QTcpSocket::stateChanged, this, &TCPHelper::handle_tcp_socket_state_changed);
        QObject::connect(socket, &QTcpSocket::readyRead, this, &TCPHelper::handle_tcp_socket_ready_read);
    }
}

TCPHelper::~TCPHelper()
{
    if(!sockets.isEmpty()){
        QTcpSocket *socket = nullptr;
        while(!sockets.isEmpty()){
            socket = sockets.first();
            socket->disconnectFromHost();
            if(socket->state() != QAbstractSocket::UnconnectedState) socket->waitForDisconnected();
            sockets.removeFirst();
            socket->deleteLater();
        }
    }

    if(server){
        if(server->isListening()){
            server->close();
            server->deleteLater();
        }
    }
    if(m_timer){
        if(m_timer->isActive()) m_timer->stop();
        m_timer->deleteLater();
    }
}

bool TCPHelper::start(const QHostAddress &address, int port)
{
    this->port = port;
    this->ip = QHostAddress(address.toIPv4Address()).toString();
    if(this->role == Server){
        if(this->server->isListening()) return false;
        return server->listen(address, port);
    }
    if(this->role == Client){
        QTcpSocket *socket = sockets.first();
        socket->abort();
        socket->connectToHost(address, port, QIODevice::ReadWrite);
        return true;
    }
    return false;
}

void TCPHelper::stop()
{
    if(this->role == Server){
        this->server->close();
    }
    if(this->role == Client){
        QTcpSocket *socket = sockets.first();
        socket->disconnectFromHost();
        if(socket->state() != QAbstractSocket::UnconnectedState) socket->waitForDisconnected(1000);
    }
}

qint64 TCPHelper::write(const char *data, qint64 len, int id)
{
    int si = sockets.count();
    if(si == 0) return -1;
    QTcpSocket *socket = nullptr;

    for(int i = 0; i < si; i++){
        socket = sockets.at(i);
        if(socket->peerPort() == id){
            break;
        }else{
            socket = nullptr;
        }
    }
    if(!socket) return -1;
    return socket->write(data, len);
}

qint64 TCPHelper::write(const char *data, int id)
{
    int len = std::strlen(data);
    return write(data, len, id);
}

void TCPHelper::setAutoWritePriod(int msec)
{
    m_priod = msec;
    if(!m_timer) return;
    bool ts = m_timer->isActive();

    if(ts) m_timer->stop();
    m_timer->setInterval(m_priod);
    if(ts) m_timer->start();
}

int TCPHelper::autoWritePriod()
{
    return m_priod;
}

void TCPHelper::startAutoWrite(const QList<int> ids, int msec)
{
    if(sockets.isEmpty()) return;

    if(!m_timer){
        m_timer = new QTimer();
        t_timeout_connect = QObject::connect(m_timer, &QTimer::timeout, this, &TCPHelper::handle_timer_timeout);
    }
    m_priod = msec;
    if(m_timer->isActive()) m_timer->stop();
    m_timer->setInterval(m_priod);

    // 查找发送连接队列
    if(!t_sockets.isEmpty()) t_sockets.clear();
    int si = sockets.count(), pos = -1;
    QTcpSocket *socket = nullptr;
    for(int i = 0; i < si; i++){
        socket = sockets.at(i);
        pos = ids.indexOf(socket->peerPort());
        if(pos >= 0){
            t_sockets.append(socket);
        }
    }

    if(!m_timer->isActive()) m_timer->start();
}

void TCPHelper::startAutoWrite(const QList<int> ids, const QByteArray &data, int msec)
{
    m_auto_data = data;
    startAutoWrite(ids, msec);
}

void TCPHelper::setAutoWrite(const QByteArray &data)
{
    m_auto_data = data;
}

void TCPHelper::stopAutoWrite()
{
    if(!m_timer) return;

    if(m_timer->isActive()) m_timer->stop();
    QObject::disconnect(t_timeout_connect);
    m_timer->deleteLater();

    if(!t_sockets.isEmpty()) t_sockets.clear();

    m_timer = nullptr;
}


void TCPHelper::handle_tcp_server_new_connection()
{
    QTcpSocket* socket = nullptr;

    while(this-server->hasPendingConnections()){

        socket = this->server->nextPendingConnection();
        if(socket == nullptr) break;
        sockets.append(socket);

        QObject::connect(socket, &QTcpSocket::disconnected, this, &TCPHelper::handle_tcp_socket_disconnected);
        QObject::connect(socket, &QTcpSocket::readyRead, this, &TCPHelper::handle_tcp_socket_ready_read);
        QObject::connect(socket, &QTcpSocket::errorOccurred, this, &TCPHelper::handle_tcp_socket_error);

//            qDebug() << "new connection:" << socket->peerAddress() << ":" << socket->peerName() << ":" << socket->peerPort();

        emit connected(this->ip, QHostAddress(socket->peerAddress().toIPv4Address()).toString(), this->port, socket->peerPort());
    }
}

void TCPHelper::handle_tcp_server_error(QAbstractSocket::SocketError socketError)
{
    int err = (int)socketError;
    emit errorOccurred(err);
}

void TCPHelper::handle_tcp_socket_connected()
{
    if(this->role == Server) return;
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    emit connected(this->ip, QHostAddress(socket->peerAddress().toIPv4Address()).toString(), this->port, socket->peerPort());
}

void TCPHelper::handle_tcp_socket_disconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    emit disconnected(this->ip, QHostAddress(socket->peerAddress().toIPv4Address()).toString(), this->port, socket->peerPort());
}

void TCPHelper::handle_tcp_socket_state_changed(QAbstractSocket::SocketState state)
{

}

void TCPHelper::handle_tcp_socket_error(QAbstractSocket::SocketError socketError)
{
    int err = (int)socketError;
    emit errorOccurred(err);
}

void TCPHelper::handle_tcp_socket_host_found()
{

}

void TCPHelper::handle_tcp_socket_ready_read()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    int len = 0, tmp = socket->bytesAvailable();
    QByteArray in;
    while(tmp){
        in.append(socket->readAll());
        len += tmp;
        tmp = socket->bytesAvailable();
    }
    emit receiveData(QHostAddress(socket->peerAddress().toIPv4Address()).toString(), this->port, socket->peerPort(), in, len);
}

void TCPHelper::handle_timer_timeout()
{
    if(t_sockets.isEmpty()) return;
    int si = t_sockets.count();
    for(int i = 0; i < si; i++){
        t_sockets.at(i)->write(m_auto_data);
    }
}
