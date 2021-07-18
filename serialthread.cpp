#include "serialthread.h"

SerialThread::SerialThread(QObject *parent) : QObject(parent)
{
    const QMutexLocker locker(&m_mutex);


//    qDebug() << "SerialThread:" << QThread::currentThread();
}

SerialThread::~SerialThread()
{
    if(port){
        port->close();
        port->deleteLater();
    }
    if(timer){
        if(timer->isActive()) timer->stop();
        timer->deleteLater();
    }
}

//void SerialThread::handleSerialError(QSerialPort::SerialPortError error)
//{
//    int err = (int)error;
////    if(! (error == QSerialPort::NoError) || (QSerialPort::TimeoutError)){
////        m_start = 0;
////    }
//    emit serialErrorOccurred(err);
//}


//void SerialThread::handleTimertimeout()
//{
//    handleSerialWrite();
//}

//void SerialThread::handleStartSend()
//{
//    if(timer && !timer->isActive()) timer->start();
//}

void SerialThread::handle_serial_changed(void)
{
    m_mutex.lock();
    if(port == nullptr) return;
    const SerialConfig config = serialConfig;

//    if((m_serialConfig.portBaud != serialConfig.portBaud) || (m_serialConfig.portName != serialConfig.portName)
//            || (m_serialConfig.portDataBit != serialConfig.portDataBit) || (m_serialConfig.portParity != serialConfig.portParity)
//            || (m_serialConfig.portStopBit != serialConfig.portStopBit) || (m_serialConfig.portFlow != serialConfig.portFlow))
//    {


//    }
    if(this->error != 0){
        if(port->isOpen()) port->close();
    }
    if(m_serialConfig.portName != config.portName){
        m_serial_changed = true;
        if(port->isOpen()){
            m_serialConfig = config;
            m_open = open_serial(config);
        }else{
            port->setPortName(config.portName);
        }
        m_serial_changed = false;
    }
    if(m_serialConfig.portBaud != config.portBaud){
        port->setBaudRate(config.portBaud);
    }
    if(m_serialConfig.portDataBit != config.portDataBit){
        port->setDataBits(config.portDataBit);
    }
    if(m_serialConfig.portParity != serialConfig.portParity){
        port->setParity(config.portParity);
    }
    if(m_serialConfig.portStopBit != serialConfig.portStopBit){
        port->setStopBits(config.portStopBit);
    }
    if(m_serialConfig.portFlow != serialConfig.portFlow){
        port->setFlowControl(config.portFlow);
    }
    if(this->error != 0){
        if(!port->isOpen()) port->open(QIODevice::ReadWrite);
    }
    m_mutex.unlock();
//    end_section:
//    m_mutex.unlock();
    //    qDebug() << "SerialThread::handle_serial_changed:" << QThread::currentThread();
}


void SerialThread::handle_serial_send(const QByteArray &data, int len)
{
    m_mutex.lock();
    if(port == nullptr){ m_mutex.unlock(); return; }
    if(m_open == false){ m_mutex.unlock(); return; }
    if(m_start == false){ m_mutex.unlock(); return; }
    const QString dStr = QString::fromUtf8(data.data(), len);

    m_sendList.append(dStr);
    m_mutex.unlock();

    if(!m_send && timer && !timer->isActive()) timer->start();
//    handleStartSend();
//    qDebug() << "SerialThread::handle_serial_send:" << QThread::currentThread();
}

void SerialThread::handle_serial_open(void)
{
    m_mutex.lock();

    m_serialConfig = serialConfig;

    m_open = open_serial(m_serialConfig);
    m_serial_changed = false;
    m_start = true;

    m_mutex.unlock();
//    qDebug() << "SerialThread::handle_serial_open:" << QThread::currentThread();
}

void SerialThread::handle_serial_start()
{
    m_mutex.lock();
    if(m_open){
        if(timer && !timer->isActive()) timer->start(1);
        m_start = true;
    }else{
        handle_serial_open();
    }
    m_mutex.unlock();
//    qDebug() << "SerialThread::handle_serial_start:" << QThread::currentThread();
}

void SerialThread::handle_serial_close()
{
    m_mutex.lock();
    if(timer && timer->isActive()) timer->stop();
    if(port && port->isOpen()){
        port->close();
    }
    if(!m_sendList.isEmpty()) m_sendList.clear();
    m_open = false;
    m_start = false;
    m_mutex.unlock();
    emit serialErrorOccurred(0);
//    qDebug() << "SerialThread::handle_serial_close:" << QThread::currentThread();
}

void SerialThread::handle_serial_pause()
{
    m_mutex.lock();
    if(timer && timer && timer->isActive()) timer->stop();
    m_start = false;
    m_mutex.unlock();
//    qDebug() << "SerialThread::handle_serial_pause:" << QThread::currentThread();
}

bool SerialThread::open_serial(SerialConfig config)
{
    if(port){
//        QObject::disconnect(port, &QSerialPort::readyRead, this, &SerialThread::handleSerialRead);
//        QObject::disconnect(port, &QSerialPort::errorOccurred, this, &SerialThread::handleSerialError);
        if(port->isOpen()) port->close();
//        port->deleteLater();
    }else{
        port = new QSerialPort(config.portName, this);
        QObject::connect(port, &QSerialPort::readyRead, this, &SerialThread::handleSerialRead);
        QObject::connect(port, &QSerialPort::errorOccurred, this, &SerialThread::handleSerialError);

        //TODO 连接Serial信号与槽
        QObject::connect(port, &QSerialPort::baudRateChanged, this, &SerialThread::handleSerialBaudRateChanged);
        QObject::connect(port, &QSerialPort::dataBitsChanged, this, &SerialThread::handleSerialDataBitsChanged);
        QObject::connect(port, &QSerialPort::flowControlChanged, this, &SerialThread::handleSerialFlowControlChanged);
        QObject::connect(port, &QSerialPort::parityChanged, this, &SerialThread::handleSerialParityChanged);
        QObject::connect(port, &QSerialPort::stopBitsChanged, this, &SerialThread::handleSerialStopBitsChanged);
    }

    port->setPortName(config.portName);
    port->setBaudRate(config.portBaud);
    port->setDataBits(config.portDataBit);
    port->setFlowControl(config.portFlow);
    port->setParity(config.portParity);
    port->setStopBits(config.portStopBit);
    port->setReadBufferSize(1024);

    bool res = false;
    res = port->open(QIODevice::ReadWrite);
    if(!res) return res;

    if(timer){
        if(!timer->isActive()){
            timer->start();
        }
    }else{
        timer = new QTimer(this);
        QObject::connect(timer, &QTimer::timeout, this, &SerialThread::handleTimertimeout);
        timer->start();
    }
    return res;
}

void SerialThread::handleSerialWrite()
{
//    qDebug() << "SerialThread::handleSerialWrite:" << QThread::currentThread();
    m_mutex.lock();
    if(m_send){ m_mutex.unlock(); return; }
    if(m_start == false){ if(timer && timer->isActive()){ timer->stop(); } m_mutex.unlock(); return; }
    if(port == nullptr){ m_mutex.unlock(); return; }
    if(!port->isOpen()){ m_mutex.unlock(); return; }
    if(m_sendList.isEmpty()){
        if(timer && timer->isActive()) timer->stop();
        m_mutex.unlock(); return;
    }

    m_send = true;
    if(m_serial_changed){
//        bool b = open_serial(m_serialConfig);
//        m_serial_changed = false;
//        if(b == false) return;
        return;
    }
    const QByteArray sd = m_sendList.first().toUtf8();
    m_sendList.removeFirst();

//    qDebug() << "handleSerialWrite:" << QString::fromUtf8(sd);

    if(timer && timer->isActive()) timer->stop();


    port->write(sd);
//    port->waitForBytesWritten(3000);

    m_send = false;

    if(timer && (!m_sendList.isEmpty()) && (!timer->isActive())) timer->start();  // 发送队列非空, 启动下一次传输
    if(timer && (m_sendList.isEmpty()) && (timer->isActive())) timer->stop();

    m_mutex.unlock();
}

void SerialThread::handleSerialRead()
{
    int len = 0, tmp = 0;

    tmp = port->bytesAvailable();
    if(tmp <= 0) return;
    QByteArray rec;
    while(!port->atEnd()){
        rec += port->readAll();
        len += tmp;
        tmp = port->bytesAvailable();
    }
//    qDebug() << "handleSerialRead:" << QString::fromUtf8(rec);
    emit serialdataRecieveComplete(rec, len);

    m_mutex.lock();
    if(m_serial_changed){
        bool b = open_serial(m_serialConfig);
        m_serial_changed = false;
        if(b == false) return;
    }
    m_mutex.unlock();
}

void SerialThread::threadInitConnect()
{

}
