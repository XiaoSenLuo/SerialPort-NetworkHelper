#ifndef UDPHELPER_H
#define UDPHELPER_H

#include <QObject>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QTimer>
#include <QNetworkInterface>

class UDPHelper : public QObject
{
    Q_OBJECT
public:

    enum Role{ Client = 0, Server = 1};

    explicit UDPHelper(int role, QObject *parent = nullptr);
    UDPHelper(int role, const QHostAddress& address, int port, QObject* parent = nullptr);
    ~UDPHelper();

    bool start(const QHostAddress &address, int port);
    void stop();

    qint64 write(const char *data, qint64 len, const QHostAddress& address, int port);
    qint64 write(const char *data, const QHostAddress& address, int port);
    inline qint64 write(const QByteArray &data, const QHostAddress& address, int port)
    { return write(data.constData(), data.size(), address, port); }

    void setAutoWritePriod(int msec = 0);
    int autoWritePriod(void);
    void setAutoWrite(const QByteArray &data);
    void startAutoWrite(int msec = 0);
    void startAutoWrite(const QByteArray &data, int msec = 0);
    void stopAutoWrite(void);

signals:
    void errorOccurred(int err);

//    void readyRead(void);
    void receiveData(const QString &ip, int port, int id, const QByteArray& data, int len);

private slots:
    void handle_udp_socket_error(QAbstractSocket::SocketError socketError);

    void handle_udp_socket_ready_read(void);

    void handle_timer_timeout(void);

private:
    QUdpSocket *udp_socket = nullptr;

    int port;
    QHostAddress address;
    int role = Client;

    QTimer *m_timer = nullptr;
    int m_priod = 0;
    QByteArray m_auto_data;
    QMetaObject::Connection t_timeout_connect;

};

#endif // UDPHELPER_H
