#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QString>
#include <QTimer>
#include <QList>
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QDateTime>
#include <QActionGroup>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QThread>
#include <QImage>
#include <QIcon>
#include <QPixmap>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QNetworkAddressEntry>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QStandardItem>
#include <QStandardItemModel>

#include "main.h"
#include "settingdialog.h"
#include "ttkmarqueelabel.h"
#include "serialhelper.h"
#include "tcphelper.h"
#include "udphelper.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#include <QSerialPort>
#include <QSerialPortInfo>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    enum {TAB_SERIAL = 0, TAB_NETWORK = 1};

    const QString ui_getSendData(void);

    void ui_setSendData(const QString &str);
    void ui_clearSendData(void);
    void serial_send(const QString &data, int len);
    void serial_send(void);

    void ui_start_resend(void);

    void ui_showMessage(const QString &message, int time = 0, QColor color = Qt::darkGreen);

    enum DEVICE_STATUS{STATUS_CLOSE = -1, STATUS_OPEN = 0, STATUS_PAUSE = 1, STATUS_OCCUR_ERROR = 2};

    // Serial
    int ui_serial_getPortNumber(void);
    const QString ui_serial_getPortName(void);
    qint64 ui_serial_getBaud(void);
    QSerialPort::DataBits ui_serial_getDataBit(void);
    QSerialPort::Parity ui_serial_getParity(void);

    QSerialPort::StopBits ui_serial_getStopBit(void);
    QSerialPort::FlowControl ui_serial_getFlow(void);

    // Recieve
    int ui_recieve_getRecieveMode(void);

    bool ui_show_isEnableAutoNewLine(void);

    bool ui_show_isEnableShowSend(void);

    bool ui_show_isEnableShowTime(void);
    bool ui_isEnableBufferMode(void);
    bool ui_show_isEnableShowColor(void);
    int ui_recvieve_getBufferSize(void);

    // Send
    int ui_send_getSendMode(void);

    bool ui_send_isEnableAutoRepeat(void);
    void ui_send_setAutoRepeatState(bool set);
    int ui_send_getRepeatTime(void);

    int ui_send_getRepeatTimeUnit(void);

    // LOG
    const QString ui_log_getLogPath();
    bool ui_log_isEnableLog(void);


public slots:

    void handle_serial_recieve_data(const QByteArray &data, int len);
    void handle_serial_error(int error);


#ifdef NETWORK_THREAD
    void handle_tcp_server_occur_error(int error){

    }

    void handle_tcp_server_new_connection(const QString& clientip, int id){

    }

    void handle_tcp_server_client_disconnected(const QString& clientip, int id){

    }

    void handle_tcp_server_started(const QString& serverip, int port){
        qDebug() << "TCPServer Started:" << serverip << ":" << port;

    }

    void handle_tcp_server_recieved_data(const QByteArray &data, int len, const QString& sourceip, int id){
        ui_showRecieveData(data, len);
    }

    void handle_tcp_client_recieved_data(const QByteArray &data, int len, const QString& sourceip, int id){
        ui_showRecieveData(data, len);
    }

    void handle_tcp_client_occur_error(int err){

    }

    void handle_tcp_client_connected(void){

    }

    void handle_tcp_client_disconnected(void){

    }
#endif

private slots:

    void handle_serialhelper_readyread(void);

    void handle_ui_line_mode_changed(int index);
    void handle_ui_line_mode_changed(const QString& perf);
    // setting
    void handle_setting_changed(SettingConfig config);

signals:
    void ui_serial_config_changed();

    void ui_serial_open(void);
    void ui_serial_close(void);
    void ui_serial_start(void);
    void ui_serial_pause(void);
    void ui_serial_send(const QByteArray& data, int len);

    void ui_tcp_start(const QString& ip, int port, int role);
    void ui_tcp_stop(int role);
    void ui_tcp_send(const QByteArray& data, int len, int role, const QString& desip, int id = -1);


private:
    QLabel *slabel;
    QLabel *lbTxBytes;
    QLabel *lbRxBytes;
    TTKMarqueeLabel *lbLogPath;
//    TTKMarqueeLabel *sttklabel;

    Ui::MainWindow *ui;
    QTimer refresh_port_timer;
    QTimer resend_timer;

    SerialHelper *s_helper = nullptr;         // 串口助手
    TCPHelper *tcp_helper = nullptr;
    UDPHelper *udp_helper = nullptr;

//    bool isOpen = false;

    int serialStatus = STATUS_CLOSE;
//    int openStatus = STATUS_CLOSE;
    int networkStatus = STATUS_CLOSE;

    int currentTab = 0;

    bool isAutorefresh = false;

    SettingConfig settingConfig;
    QMutex m_mutex;

    ulong txBytes = 0;
    ulong rxBytes = 0;

    QByteArray fileBuffer = QByteArray(settingConfig.logConfig.bufferSize * 1024, '\0');

    QByteArray recieveBuffer;

//    QStandardItemModel *clientModel = nullptr;
    QList<int> seleteRows;


    void ui_initSetting(SettingConfig* config);
    void ui_initConnect(void);
    void ui_creatStatusBar(void);
    void ui_creatToolBar(void);

    void ui_refreshPort(void);
    void ui_refreshNetInterface(void);

    void apply_ui_serial_config(const SerialConfig& config);

    void update_ui_serial_config(void);

    void ui_statusbar_showMessage(const QString& str, QColor color = Qt::darkGreen);
    void ui_statusbar_showRxBytes(ulong bytes);
    void ui_statusbar_showTxBytes(ulong bytes);
    void ui_statusbar_showLogPath(const QString & str);

    int ui_getCurrentTab(void);

    void ui_log_setLogPath(const QString& p);

    void ui_log_setSaveLogState(bool save);

    void ui_log_logSaveToFile(const QString &str);
    void ui_log_seleteLogPath(void);

    const SerialConfig ui_serial_getConfig(void);

    const QString inline ui_serial_makePortName(const QSerialPortInfo info){
        return QString::fromUtf8("[%1]%2").arg(info.portName()).arg(info.description());
    }
    void ui_serial_addPortName(const QSerialPortInfo info);
    void ui_serial_addPortNames(const QList<QSerialPortInfo> list);
    void ui_serial_clearPortNameExceptIndex(int index = -1);

    void ui_serial_clearPortNameExceptCurrent(void);

    void ui_serial_setPortName(QSerialPortInfo info);
    void ui_serial_setBaud(qint64 baud);
    void ui_serial_setDataBit(QSerialPort::DataBits d);
    void ui_serial_setParity(QSerialPort::Parity p);
    void ui_serial_setStopBit(QSerialPort::StopBits s);
    void ui_serial_setFlow(QSerialPort::FlowControl f);

    void ui_serial_setConfig(SerialConfig config);

    void ui_serial_toggle_pbtSend(bool _isOpen);

    // 网络
    void ui_net_role_function(int protocol);

    const QString inline ui_net_makeInterfaceStr(QNetworkInterface interface){
//         if(interface.isValid()) return QString("");
         const QString str = QString::fromUtf8("%1 [%2]").arg(interface.humanReadableName()).arg(interface.hardwareAddress());
         return str;
    }

    void ui_net_addInterface(QNetworkInterface interface);

    const QString ui_net_getCurrentInterfaceHardAddr(void);

    const QString ui_net_getCurrentInterfaceHumanNamme(void);

    const QString ui_net_getCurrentInterfaceAddr(bool ipv6 = false);

    void ui_net_setCurrentInterface(QNetworkInterface interface);

    QNetworkInterface ui_net_getInterface(bool *ok);

    bool ui_net_isEnableIPV6(void);

    int ui_net_getRole(void);

    int ui_net_getProfile(void);

    int ui_net_getPort(void);

    const QString ui_net_getIP(void);

    void ui_net_setIP(const QString &ip);


    // Tabel View

    void ui_net_initStatusTable(void);
    void ui_net_addConnectionToTable(const QString& source_ip, const QString& des_ip, int status, int port);

    void ui_showTime();
    void ui_showSend(const QString &str, bool t = false);
    void ui_showRecieve(const QString &str, bool t = false);
    void ui_clearRecieve(void);
    void ui_showRecieveData(const QByteArray& data, int len);
//    void ui_showSendData(const QByteArray& data, int len);

    void ui_addSendHistory(const QString &str);
    void ui_addSendHistory(const QStringList& list);
    void ui_clearSendHistory(void);

    void ui_recieve_initRecieveFontColor(void);

    QColor ui_recieve_getRecieveFontColor(void);
    void ui_recieve_setRecieveFontColorState(bool state);

    void ui_setShowPlaintFont(const QFont &font);


protected:
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *event) override;
    void enterEvent(QEvent *event) override;
    void actionEvent(QActionEvent *event) override;
//    bool eventFilter() override;
    // Status Bar

    //    void ui_showMessage(const QString & m);
};

#endif // MAINWINDOW_H
