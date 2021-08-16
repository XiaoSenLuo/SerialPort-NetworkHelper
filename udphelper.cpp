#include "udphelper.h"

UDPHelper::UDPHelper(int role, QObject *parent) : QObject(parent)
{
    udp_socket = new QUdpSocket(this);
    this->role = role;

    QObject::connect(udp_socket, &QUdpSocket::readyRead, this, &UDPHelper::handle_udp_socket_ready_read);
    QObject::connect(udp_socket, &QUdpSocket::errorOccurred, this, &UDPHelper::handle_udp_socket_error);
}

UDPHelper::UDPHelper(int role, const QHostAddress &address, int port, QObject *parent)
 : QObject(parent)
{
    udp_socket = new QUdpSocket(this);

    QObject::connect(udp_socket, &QUdpSocket::readyRead, this, &UDPHelper::handle_udp_socket_ready_read);
    QObject::connect(udp_socket, &QUdpSocket::errorOccurred, this, &UDPHelper::handle_udp_socket_error);

    this->role = role;
    this->port = port;
    this->address = address;

    if(UDPHelper::Server == role){
        udp_socket->bind(address, port);
    }
}

UDPHelper::~UDPHelper()
{
    if(m_timer){
        if(m_timer->isActive()) m_timer->stop();
        m_timer->deleteLater();
    }
    stop();
    udp_socket->deleteLater();
}

bool UDPHelper::start(const QHostAddress &address, int port)
{
    this->port = port;
    this->address = address;
    if(this->role == Server){
        return udp_socket->bind(address, port);
    }
    return false;
}

void UDPHelper::stop()
{
    if(this->role == UDPHelper::Server){
        udp_socket->close();
    }
}

qint64 UDPHelper::write(const char *data, qint64 len, const QHostAddress &address, int port)
{
    return udp_socket->writeDatagram(data, len, address, port);
}

qint64 UDPHelper::write(const char *data, const QHostAddress &address, int port)
{
    int len = std::strlen(data);
    return udp_socket->writeDatagram(data, len, address, port);
}

void UDPHelper::setAutoWritePriod(int msec)
{
    m_priod = msec;
    if(!m_timer) return;
    bool ts = m_timer->isActive();

    if(ts) m_timer->stop();
    m_timer->setInterval(m_priod);
    if(ts) m_timer->start();
}

int UDPHelper::autoWritePriod()
{
    return m_priod;
}

void UDPHelper::startAutoWrite(int msec)
{
    if(!m_timer){
        m_timer = new QTimer();
        t_timeout_connect = QObject::connect(m_timer, &QTimer::timeout, this, &UDPHelper::handle_timer_timeout);
    }
    m_priod = msec;
    if(m_timer->isActive()) m_timer->stop();
    m_timer->setInterval(m_priod);

    if(!m_timer->isActive()) m_timer->start();
}

void UDPHelper::startAutoWrite(const QByteArray &data, int msec)
{
    m_auto_data = data;
    startAutoWrite(msec);
}

void UDPHelper::setAutoWrite(const QByteArray &data)
{
    m_auto_data = data;
}

void UDPHelper::stopAutoWrite()
{
    if(!m_timer) return;
    if(m_timer->isActive()) m_timer->stop();
    QObject::disconnect(t_timeout_connect);
    m_timer->deleteLater();
    m_timer = nullptr;
}

void UDPHelper::handle_udp_socket_error(QAbstractSocket::SocketError socketError)
{
    int err = (int)socketError;
    emit errorOccurred(err);
}

void UDPHelper::handle_udp_socket_ready_read()
{
    QUdpSocket *socket = qobject_cast<QUdpSocket*>(sender());
    QNetworkDatagram datagram = socket->receiveDatagram();

    const QString sender_ip = QHostAddress(datagram.senderAddress().toIPv4Address()).toString();
    int port = datagram.senderPort();
    const QByteArray data = datagram.data();

    emit receiveData(sender_ip, port, -1, data, data.size());
}

void UDPHelper::handle_timer_timeout()
{
    write(m_auto_data, this->address, this->port);
}
