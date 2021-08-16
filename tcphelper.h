#ifndef TCPHELPER_H
#define TCPHELPER_H

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
#include <QVector>

class TCPHelper : public QObject
{
    Q_OBJECT
public:

    enum Role{ Client = 0, Server = 1};

    explicit TCPHelper(int role, QObject *parent = nullptr);

    ~TCPHelper();

    bool start(const QHostAddress &address, int port);
    void stop();


    qint64 write(const char *data, qint64 len, int id);
    qint64 write(const char *data, int id);
    inline qint64 write(const QByteArray &data, int id)
    { return write(data.constData(), data.size(), id); }

    void setAutoWritePriod(int msec = 0);
    int autoWritePriod(void);
    void setAutoWrite(const QByteArray &data);
    void startAutoWrite(const QList<int> ids, int msec = 0);
    void startAutoWrite(const QList<int> ids, const QByteArray &data, int msec = 0);
    void stopAutoWrite(void);


signals:

    void connected(const QString &source_ip, const QString &des_ip, int port, int id);
    void disconnected(const QString &source_ip, const QString &des_ip, int port, int id);
    void stateChanged(int new_state);
    void errorOccurred(int err);
//    void readyRead(void);
    void receiveData(const QString &ip, int port, int id, const QByteArray& data, int len);

private slots:

    void handle_tcp_server_new_connection(void);
    void handle_tcp_server_error(QAbstractSocket::SocketError socketError);

    void handle_tcp_socket_connected(void);
    void handle_tcp_socket_disconnected(void);
    void handle_tcp_socket_state_changed(QAbstractSocket::SocketState state);
    void handle_tcp_socket_error(QAbstractSocket::SocketError socketError);
    void handle_tcp_socket_host_found(void);

    void handle_tcp_socket_ready_read(void);

    void handle_timer_timeout(void);
private:

    QTcpServer *server = nullptr;
    QVector<QTcpSocket *> sockets;

    int port;
    QString ip;
    int max_connections = 1;
    int role = Client;

    QTimer *m_timer = nullptr;
    int m_priod = 0;
    QByteArray m_auto_data;
    QMetaObject::Connection t_timeout_connect;
    QVector<QTcpSocket*> t_sockets;

    void initConnect(void);

};

#endif // TCPHELPER_H
