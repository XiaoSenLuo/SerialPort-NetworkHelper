#ifndef SERIALHELPER_H
#define SERIALHELPER_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QString>
#include <QTimer>
#include <QFile>
#include <QDataStream>
#include <QSerialPort>
#include <QSerialPortInfo>


class SerialHelper : public QSerialPort
{
    Q_OBJECT
public:
    explicit SerialHelper(QObject *parent = nullptr);
    explicit SerialHelper(const QSerialPortInfo &info, QObject *parent = nullptr);
    explicit SerialHelper(const QString &name, QObject *parent = nullptr);
    ~SerialHelper();


    QSerialPort* port(void);
    void setPort(const QSerialPortInfo &info);

    bool open(OpenMode mode) override;
    void close() override;

    bool isOpen() const;
    bool isSequential() const override;

    qint64 bytesAvailable() const override;
    qint64 bytesToWrite() const override;
    bool canReadLine() const override;

//    bool waitForReadyRead(int msecs = 30000) override;
//    bool waitForBytesWritten(int msecs = 30000) override;


    void setPortName(const QString &name);
    QString portName() const;
    bool setBaudRate(qint32 baudRate, Directions directions = AllDirections);
    qint32 baudRate(Directions directions = AllDirections) const;

    bool setDataBits(DataBits dataBits);
    DataBits dataBits() const;

    bool setParity(Parity parity);
    Parity parity() const;

    bool setStopBits(StopBits stopBits);
    StopBits stopBits() const;

    bool setFlowControl(FlowControl flowControl);
    FlowControl flowControl() const;

    bool setDataTerminalReady(bool set);
    bool isDataTerminalReady();

    bool setRequestToSend(bool set);
    bool isRequestToSend();

    PinoutSignals pinoutSignals();

    bool flush();
    bool clear(Directions directions = AllDirections);

    qint64 readBufferSize() const;
    void setReadBufferSize(qint64 size);

    qint64 write(const char *data, qint64 len);
    qint64 write(const char *data);
    inline qint64 write(const QByteArray &data)
    { return write(data.constData(), data.size()); }

    qint64 read(char *data, qint64 maxlen);
    QByteArray read(qint64 maxlen);
    QByteArray readAll();
    qint64 readLine(char *data, qint64 maxlen);
    QByteArray readLine(qint64 maxlen = 0);

    // <------------------------------>
    void setAutoWritePriod(int msec = 0);
    int autoWritePriod(void);
    void setAutoWrite(const QByteArray &data);
    void startAutoWrite(int msec = 0);
    void stopAutoWrite(void);

    template <typename ... Args>
    QMetaObject::Connection callOnReadyRead(Args && ...args){
        if(!m_port) return QMetaObject::Connection();
//        s_connect = QObject::connect(m_port, &QSerialPort::readyRead, std::forward<Args>(args)...);
//        return s_connect;
        return QObject::connect(m_port, &QSerialPort::readyRead, std::forward<Args>(args)...);
    }

signals:
    void errorOccurred(int err);

protected:

//    qint64 readData(char *data, qint64 maxSize) override;
//    qint64 readLineData(char *data, qint64 maxSize) override;
//    qint64 writeData(const char *data, qint64 maxSize) override;

private:
    QSerialPort *m_port = nullptr;

//    QMetaObject::Connection s_connect;
    QMetaObject::Connection s_err_connect;

//    QByteArray rec_buffer, rec_buffer_copy;  // 双缓存

    QTimer *m_timer = nullptr;
    int m_priod = 0;
    QByteArray m_auto_data;
    QMetaObject::Connection t_timeout_connect;

    void handle_QSerialPort_errorOccurred(QSerialPort::SerialPortError error);

    void handle_QTimer_timeout(void);
};


#endif // SERIALHELPER_H
