#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QObject>
#include <QList>
#include <QByteArray>
#include <QString>
#include <QTimer>
#include <QFile>
#include <QDataStream>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "main.h"

class SerialThread : public QObject
{
    Q_OBJECT
public:
    explicit SerialThread(QObject *parent = nullptr);

    ~SerialThread();

signals:
//    void serialSendFinished(void);
//    void serialRecieveFinished(void);
    void serialErrorOccurred(int error);
    void serialdataRecieveComplete(const QByteArray &data, int len);

    void serialBaudRateChanged(int baudRate, int directions);
    void serialDataBitsChanged(int dataBits);
    void serialFlowControlChanged(int flow);
    void serialParityChanged(int parity);
    void serialStopBitsChanged(int stopBits);

private slots:
    void handleSerialError(QSerialPort::SerialPortError error){
        int err = (int)error;
        this->error = err;
        switch(error){
        case QSerialPort::NoError:
            break;
        case QSerialPort::TimeoutError:
            break;
        default:
            if(!m_sendList.isEmpty()){
                m_sendList.clear();
            }
            if(m_send) m_send = false;
            if(m_start) m_start = false;
        }

        emit serialErrorOccurred(err);
    }
    void handleSerialBaudRateChanged(qint32 baudRate, QSerialPort::Directions directions){
        emit serialBaudRateChanged((int)baudRate, (int)directions);
    }
    void handleSerialDataBitsChanged(QSerialPort::DataBits dataBits){
        qDebug() << dataBits;
        emit serialDataBitsChanged((int)dataBits);
    }
    void handleSerialFlowControlChanged(QSerialPort::FlowControl flow){
        qDebug() << flow;
        emit serialFlowControlChanged((int)flow);
    }
    void handleSerialParityChanged(QSerialPort::Parity parity){
        qDebug() << parity;
        emit serialParityChanged((int)parity);
    }
    void handleSerialStopBitsChanged(QSerialPort::StopBits stopBits){
        qDebug() << stopBits;
        emit serialStopBitsChanged((int)stopBits);
    }

    void handleTimertimeout(void){
        handleSerialWrite();
    }
    void handleStartSend(void){
        if(timer && !timer->isActive()) timer->start();
    }

public slots:
    void handle_serial_changed(void);
//    void handle_serial_changed(const QString &newport, const int newbuad, const int newdatabit, const int newparity, const int newstopbit, const int newflow);


    void handle_serial_send(const QByteArray& data, int len);
    void handle_serial_open(void);
    void handle_serial_start(void);
    void handle_serial_close(void);
    void handle_serial_pause(void);

private:
    QSerialPort *port = nullptr;
    QTimer *timer = nullptr;

    QMutex m_mutex;

    bool m_send = false;
    bool m_serial_changed = false;
    bool m_start = false;
    bool m_open = false;
//    bool m_pause = false;

    int error = 0;

    SerialConfig m_serialConfig;

    QList<QString> m_sendList;

    bool open_serial(SerialConfig config);
    void handleSerialWrite(void);
    void handleSerialRead(void);
    void threadInitConnect(void);
};

#endif // SERIALTHREAD_H
