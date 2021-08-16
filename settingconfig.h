#ifndef SETTINGCONFIG_H
#define SETTINGCONFIG_H


#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFont>
#include <QColor>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include <QList>


class SerialConfig{
public:
    const char* parityStr[6] = {"None", "", "Even", "Odd", "Space", "Mark"};
    const char* flowStr[3] = {"None", "RTS/CTS", "XON/XOFF"};
    const char* stopStr[4] = {"", "1", "2", "1.5"};
    const char* errorStr[14] = {"NoError", "DeviceNotFoundError", "PermissionError", "OpenError", "ParityError", "FramingError",
                               "BreakConditionError", "WriteError", "ReadError", "ResourceError", "UnsupportedOperationError",
                                "UnknownError", "TimeoutError", "NotOpenError"};
    QString portName;
    qint32 portBaud;
    QSerialPort::DataBits portDataBit;
    QSerialPort::Parity portParity;
    QSerialPort::StopBits portStopBit;
    QSerialPort::FlowControl portFlow;

    SerialConfig(){
        portBaud = QSerialPort::Baud115200;
        portDataBit = QSerialPort::Data8;
        portParity = QSerialPort::NoParity;
        portStopBit = QSerialPort::OneStop;
        portFlow = QSerialPort::NoFlowControl;
    }
    ~SerialConfig(){

    }
};

enum CHAR_MODE {ASCII_MODE = 0, HEX_MODE = 16};

enum LINE_MODE {WIN_MODE = 0, UNIX_MODE = 1, MACOS_MODE = 2, MACOS_LEGACY_MODE = 3};

class ReceiveSettingConfig{
public:
    int showMode;
    int lineMode;
    bool bufferMode;
    int bufferSize;
    ReceiveSettingConfig(){
        showMode = ASCII_MODE;
        bufferMode = false;
        bufferSize = 128;
    }
};

class SendSettingConfig{
public:
    int sendMode;
    bool enableAutoResend;
    int ResendTime;
    int ResendUnit;

    SendSettingConfig(){
       sendMode = ASCII_MODE;
       enableAutoResend = false;
       ResendTime = 0;
       ResendUnit = 0;
    }
};

class LogSettingConfig{
public:
    bool enableSaveLog;
    QString filePath;
    int fileLimit;
    bool enableBuffer;
    int bufferSize;
    LogSettingConfig(){
        enableSaveLog = false;
        fileLimit = 1024;
        enableBuffer = false;
        bufferSize = 4;
    }
};

class ShowSettingConfig{
public:
    int bufferSize;
    QFont font;
    bool enableShowColor;
    QColor recColor;
    QColor sendColor;

    bool enableShowTime;
    bool enableShowSend;
    bool enableAutoNewLine;

    ShowSettingConfig(){
        enableShowColor = true;
        enableAutoNewLine = false;
        enableShowSend = true;
        enableShowTime = true;
        bufferSize = 1;
        recColor = QColor(QLatin1String("#0000FF"));
        sendColor = QColor(QLatin1String("#55AA7F"));
    }
//    ~ShowSettingConfig(){

//    }
};

class NetWorkSettingConfig{
public :
    QNetworkInterface interface;
    int netRole;
    int netProfile;
    int port;
    QString ip;

enum { Client = 0, Server = 1, Multicast = 2, Broadcast = 3};
enum { TCP = 0, UDP = 1};

    NetWorkSettingConfig(){
        ip = QString::fromUtf8("127.0.0.1");
        netRole = Client;
        netProfile = TCP;
        port = 8080;

    }

    const QString getGlobalAddress(void){
        QList<QNetworkAddressEntry> alist = interface.addressEntries();
        if(alist.isEmpty()) return QString("");
        for(int i = 0; i < alist.size(); i++){
            const QHostAddress addr = alist.at(i).ip();
            if(!addr.isNull() && addr.isGlobal()){
                return addr.toString();
            }
        }

    }
};

class SettingConfig{
public:
    const char* lineEnd[4] = {"\r\n", "\n", "\n", "\r"};

    int                  serialChanged;
    int                  lineMode;
    SerialConfig         serialConfig;
    ReceiveSettingConfig recConfig;
    SendSettingConfig    sendConfig;
    LogSettingConfig     logConfig;
    ShowSettingConfig    showConfig;
    NetWorkSettingConfig netConfig;


    SettingConfig(){
        serialChanged = 0;
        lineMode = WIN_MODE;
    }
    ~SettingConfig(){

    }
};

//SettingConfig allSettingConfig;


#endif // SETTINGCONFIG_H
