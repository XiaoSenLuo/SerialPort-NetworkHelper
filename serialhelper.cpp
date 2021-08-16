#include "serialhelper.h"
#include <QDebug>

SerialHelper::SerialHelper(QObject *parent)
{
    m_port = new QSerialPort(parent);
    s_err_connect = QObject::connect(m_port, &QSerialPort::errorOccurred, this, &SerialHelper::handle_QSerialPort_errorOccurred);
}

SerialHelper::SerialHelper(const QSerialPortInfo &info, QObject *parent)
{
    m_port = new QSerialPort(info, parent);
    s_err_connect = QObject::connect(m_port, &QSerialPort::errorOccurred, this, &SerialHelper::handle_QSerialPort_errorOccurred);
}

SerialHelper::SerialHelper(const QString &name, QObject *parent)
{
    m_port = new QSerialPort(name, parent);
    s_err_connect = QObject::connect(m_port, &QSerialPort::errorOccurred, this, &SerialHelper::handle_QSerialPort_errorOccurred);
}

SerialHelper::~SerialHelper()
{
    if(m_port){
        if(m_port->isOpen()) m_port->close();
        QObject::disconnect(s_err_connect);
        m_port->deleteLater();
    }
    if(m_timer){
        if(m_timer->isActive()) m_timer->stop();
    }
}

QSerialPort *SerialHelper::port()
{
    return this->m_port;
}

void SerialHelper::setPort(const QSerialPortInfo &info)
{
    if(m_port){
        if(m_port->isOpen()){
            m_port->close();
        }

        QObject::disconnect(s_err_connect);

        m_port->deleteLater();
    }
    m_port = new QSerialPort(info);
    s_err_connect = QObject::connect(m_port, &QSerialPort::errorOccurred, this, &SerialHelper::handle_QSerialPort_errorOccurred);
}

bool SerialHelper::open(QIODevice::OpenMode mode)
{
    if(!m_port) return false;
//    if(m_port->isOpen()) return true;

    return  m_port->open(mode);
}

void SerialHelper::close()
{
    if(!m_port) return;
    if(m_port->isOpen()) m_port->close();
//    QObject::disconnect(s_err_connect);
}

bool SerialHelper::isOpen() const
{
    if(!m_port) return false;
    return m_port->isOpen();
}

bool SerialHelper::isSequential() const
{
    if(!m_port) return false;
    return m_port->isSequential();
}

qint64 SerialHelper::bytesAvailable() const
{
    if(!m_port) return -1;
    return m_port->bytesAvailable();
}

qint64 SerialHelper::bytesToWrite() const
{
    if(!m_port) return -1;
    return m_port->bytesToWrite();
}

bool SerialHelper::canReadLine() const
{
    if(!m_port) return false;
    return m_port->canReadLine();
}

void SerialHelper::setPortName(const QString &name)
{
    if(!m_port) return;
    m_port->setPortName(name);
}

QString SerialHelper::portName() const
{
    if(!m_port) return QString();
    return m_port->portName();
}

bool SerialHelper::setBaudRate(qint32 baudRate, QSerialPort::Directions directions)
{
    if(!m_port) return false;
    return m_port->setBaudRate(baudRate, directions);
}

qint32 SerialHelper::baudRate(QSerialPort::Directions directions) const
{
    if(!m_port) return -1;
    return m_port->baudRate(directions);
}

bool SerialHelper::setDataBits(QSerialPort::DataBits dataBits)
{
    if(!m_port) return false;
    return m_port->setDataBits(dataBits);
}

QSerialPort::DataBits SerialHelper::dataBits() const
{
    if(!m_port) return QSerialPort::UnknownDataBits;
    return m_port->dataBits();
}

bool SerialHelper::setParity(QSerialPort::Parity parity)
{
    if(!m_port) return false;
    return m_port->setParity(parity);
}

QSerialPort::Parity SerialHelper::parity() const
{
    if(!m_port) return QSerialPort::UnknownParity;
    return m_port->parity();
}

bool SerialHelper::setStopBits(QSerialPort::StopBits stopBits)
{
    if(!m_port) return false;
    return m_port->setStopBits(stopBits);
}

QSerialPort::StopBits SerialHelper::stopBits() const
{
    if(!m_port) return QSerialPort::UnknownStopBits;
    return m_port->stopBits();
}

bool SerialHelper::setFlowControl(QSerialPort::FlowControl flowControl)
{
    if(!m_port) return false;
    return m_port->setFlowControl(flowControl);
}

QSerialPort::FlowControl SerialHelper::flowControl() const
{
    if(!m_port) return QSerialPort::UnknownFlowControl;
    return m_port->flowControl();
}

bool SerialHelper::setDataTerminalReady(bool set)
{
    if(!m_port) return false;
    return m_port->setDataTerminalReady(set);
}

bool SerialHelper::isDataTerminalReady()
{
    if(!m_port) return false;
    return m_port->isDataTerminalReady();
}

bool SerialHelper::setRequestToSend(bool set)
{
    if(!m_port) return false;
    return m_port->setRequestToSend(set);
}

bool SerialHelper::isRequestToSend()
{
    if(!m_port) return false;
    return m_port->isRequestToSend();
}

QSerialPort::PinoutSignals SerialHelper::pinoutSignals()
{
    if(!m_port) return QSerialPort::NoSignal;
    return m_port->pinoutSignals();
}

bool SerialHelper::flush()
{
    if(!m_port) return false;
    return m_port->flush();
}

bool SerialHelper::clear(QSerialPort::Directions directions)
{
    if(!m_port) return false;
    return m_port->clear(directions);
}

qint64 SerialHelper::readBufferSize() const
{
    if(!m_port) return -1;
    return m_port->readBufferSize();
}

void SerialHelper::setReadBufferSize(qint64 size)
{
    if(!m_port) return;
    m_port->setReadBufferSize(size);
}

qint64 SerialHelper::write(const char *data, qint64 len)
{
    if(!m_port) return -1;
    return m_port->write(data, len);
}

qint64 SerialHelper::write(const char *data)
{
    if(!m_port) return -1;
    return m_port->write(data);
}

qint64 SerialHelper::read(char *data, qint64 maxlen)
{
    if(!m_port) return -1;
    return m_port->read(data, maxlen);
}

QByteArray SerialHelper::read(qint64 maxlen)
{
    if(!m_port) return QByteArray();
    return m_port->read(maxlen);
}

QByteArray SerialHelper::readAll()
{
    if(!m_port) return QByteArray();
    return m_port->readAll();
}

qint64 SerialHelper::readLine(char *data, qint64 maxlen)
{
    if(!m_port) return -1;
    return m_port->readLine(data, maxlen);
}

QByteArray SerialHelper::readLine(qint64 maxlen)
{
    if(!m_port) return QByteArray();
    return m_port->readLine(maxlen);
}

void SerialHelper::setAutoWritePriod(int msec)
{
    m_priod = msec;
    if(!m_timer) return;
    bool ts = m_timer->isActive();

    if(ts) m_timer->stop();
    m_timer->setInterval(m_priod);
    if(ts) m_timer->start();
}

int SerialHelper::autoWritePriod()
{
    return m_priod;
}

void SerialHelper::startAutoWrite(int msec)
{
    if(!m_port) return;
    if(!m_port->isOpen()) return;

    if(!m_timer){
        m_timer = new QTimer();
        t_timeout_connect = QObject::connect(m_timer, &QTimer::timeout, this, &SerialHelper::handle_QTimer_timeout);
    }
    m_priod = msec;
    if(m_timer->isActive()) m_timer->stop();
    m_timer->setInterval(m_priod);
    if(!m_timer->isActive()) m_timer->start();
}

void SerialHelper::startAutoWrite(const QByteArray &data, int msec)
{
    m_auto_data = data;
    startAutoWrite(msec);
}

void SerialHelper::setAutoWrite(const QByteArray &data)
{
    m_auto_data = data;
}

void SerialHelper::stopAutoWrite()
{
    if(!m_timer) return;
    if(m_timer->isActive()) m_timer->stop();
    QObject::disconnect(t_timeout_connect);
    m_timer->deleteLater();
    m_timer = nullptr;
}

void SerialHelper::handle_QSerialPort_errorOccurred(QSerialPort::SerialPortError error)
{
//    qDebug() << error;
    int err = (int)error;
    emit SerialHelper::errorOccurred(err);
}

void SerialHelper::handle_QTimer_timeout()
{
    SerialHelper::write(m_auto_data);
}
