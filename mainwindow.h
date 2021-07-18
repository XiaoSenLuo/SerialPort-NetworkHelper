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

#include "main.h"
#include "settingdialog.h"
#include "ttkmarqueelabel.h"


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
    const QString ui_getSendData(void){
        return this->ui->pteSend->toPlainText();
    }

    void ui_setSendData(const QString &str){
        this->ui->pteSend->clear();
        this->ui->pteSend->insertPlainText(str);
        this->ui->pteSend->moveCursor(QTextCursor::EndOfLine);
    }
    void ui_clearSendData(void){
        this->ui->pteSend->clear();
        this->ui->pteSend->moveCursor(QTextCursor::Start);
    }
    void serial_send(const QString &data, int len);
    void serial_send(void);

    void ui_start_resend(void);

    void ui_showMessage(const QString &message, int time = 0, QColor color = Qt::darkGreen){
        //    this->ui->statusbar->showMessage(message, time);
        //    if(color.isValid()) ui->statusbar->setStyleSheet(QString::fromUtf8("QStatusBar{color:%1;}\n").arg(color.name(QColor::HexArgb)));
            ui_statusbar_showMessage(message, color);
    }

    enum DEVICE_STATUS{STATUS_CLOSE = -1, STATUS_OPEN = 0, STATUS_PAUSE = 1, STATUS_OCCUR_ERROR = 2};
public slots:
    void handle_serial_recieve_data(const QByteArray &data, int len);
//    void handle_serial_open_status(int b){ serialStatus = (b ? STATUS_OPEN : STATUS_CLOSE); ui_serial_toggle_pbtSend(serialStatus ? false : true); }
    void handle_serial_error(int error);

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

private slots:
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

    void ui_initSetting(SettingConfig* config);
    void ui_initConnect(void);
    void ui_creatStatusBar(void);
    void ui_creatToolBar(void);

    void ui_refreshPort(void);
    void ui_refreshNetInterface(void);

    void update_ui_serial_config(void);

    void ui_statusbar_showMessage(const QString& str, QColor color = Qt::darkGreen){
        this->slabel->setText(str);
        this->slabel->setStyleSheet(QString::fromUtf8("QLabel{color:%1;font:%2pt \"%3\";}").arg(color.name(QColor::HexRgb)).arg(this->font().pointSize()).arg(this->font().family()));
    }
    void ui_statusbar_showRxBytes(ulong bytes){
        this->lbRxBytes->setText(QString::fromUtf8("%1 Bytes").arg(bytes));
    }
    void ui_statusbar_showTxBytes(ulong bytes){
        this->lbTxBytes->setText(QString::fromUtf8("%1 Bytes").arg(bytes));
    }
    void ui_statusbar_showLogPath(const QString & str){
        this->lbLogPath->setText(str);
        this->lbLogPath->setToolTip(str);
    }

    int ui_getCurrentTab(void){
        return this->ui->tabWidget->currentIndex();
    }

    void ui_log_setLogPath(const QString& p){
        ui_statusbar_showLogPath(p);
        this->ui->leLogPath->setText(p);
        this->ui->leLogPath->setToolTip(p);
        this->ui->leLogPath->setWhatsThis(QString::fromUtf8("日志文件路径"));
    }

    void ui_log_setSaveLogState(bool save){
        this->ui->cbEnableLog->setCheckable(save);
    }

    void ui_log_logSaveToFile(const QString &str);
    void ui_log_seleteLogPath(void);

    const SerialConfig ui_serial_getConfig(void){
        SerialConfig config;
        config.portName = ui_serial_getPortName();
        config.portDataBit = ui_serial_getDataBit();
        config.portFlow = ui_serial_getFlow();
        config.portBaud = ui_serial_getBaud();
        config.portParity = ui_serial_getParity();
        config.portStopBit = ui_serial_getStopBit();
        return config;
    }

    const QString inline ui_serial_makePortName(const QSerialPortInfo info){
        return QString::fromUtf8("[%1]%2").arg(info.portName()).arg(info.description());
    }
    void ui_serial_addPortName(const QSerialPortInfo info){
        this->ui->cbbPort->addItem(ui_serial_makePortName(info));
    }
    void ui_serial_addPortNames(const QList<QSerialPortInfo> list){
        for(int i = 0; i < list.size(); i++){
            this->ui->cbbPort->addItem(QString::fromUtf8("[%1]%2").arg(list.at(i).portName()).arg(list.at(i).description()));
        }
    }
    void ui_serial_clearPortNameExceptIndex(int index = -1){
        // TODO

        if(index < 0){
            this->ui->cbbPort->clear();
            return;
        }

        int ps = this->ui->cbbPort->count();
        if(index > (ps - 1)) return;
        int ri = ((index - 1) >= 0) ? (index - 1) : (index + 1);
        while((ri >= 0) && (ri < (this->ui->cbbPort->count()))){
            this->ui->cbbPort->removeItem(ri);
            index -= 1;
            if(index >= 0) ri = ((index - 1) >= 0) ? (index - 1) : (index + 1);
            else ri = 1;

        }
    }

    void ui_serial_clearPortNameExceptCurrent(void){
        while(this->ui->cbbPort->count() > 1){
            this->ui->cbbPort->removeItem(1);
        }
    }

    void ui_serial_setPortName(QSerialPortInfo info){
        const QString pn = QString::fromUtf8("[%1]%2").arg(info.portName()).arg(info.description());
        this->ui->cbbPort->setCurrentText(pn);
    }
    void ui_serial_setBaud(qint64 baud){
        this->ui->cbbBaud->setCurrentText(QString::fromUtf8("%1").arg(baud));
    }
    void ui_serial_setDataBit(QSerialPort::DataBits d){ this->ui->cbbDataBit->setCurrentIndex(d - this->ui->cbbDataBit->itemText(0).toInt()); }
    void ui_serial_setParity(QSerialPort::Parity p){
        int pi = (int)p;
        if(pi > 0) pi -= 1;
        this->ui->cbbParity->setCurrentIndex(pi);
    }
    void ui_serial_setStopBit(QSerialPort::StopBits s){ this->ui->cbbStop->setCurrentIndex(s - 1); }
    void ui_serial_setFlow(QSerialPort::FlowControl f){this->ui->cbbFlow->setCurrentIndex(f); }

    void ui_serial_setConfig(SerialConfig config){
        QSerialPortInfo info(config.portName);
        ui_serial_setPortName(info);
        ui_serial_setBaud(config.portBaud);
        ui_serial_setDataBit(config.portDataBit);
        ui_serial_setParity(config.portParity);
        ui_serial_setStopBit(config.portStopBit);
        ui_serial_setFlow(config.portFlow);
    }

    void ui_serial_toggle_pbtSend(bool _isOpen){
        if(_isOpen){
            this->ui->pbtSend->setText(QString::fromUtf8("发送"));
        }else{
            this->ui->pbtSend->setText(QString::fromUtf8("打开"));
        }
    }

    // 网络
    const QString ui_net_makeInterfaceStr(QNetworkInterface interface){
//         if(interface.isValid()) return QString("");
         const QString str = QString::fromUtf8("%1 [%2]").arg(interface.humanReadableName()).arg(interface.hardwareAddress());
         return str;
    }

    void ui_net_addInterface(QNetworkInterface interface){
        this->ui->cbbNetworkInterface->addItem(ui_net_makeInterfaceStr(interface));
    }

    const QString ui_net_getCurrentInterfaceHardAddr(void){
        int ns = this->ui->cbbNetworkInterface->count();
        if(ns == 0) return QString("");
        const QString str = this->ui->cbbNetworkInterface->currentText();
        int pos = str.indexOf('[');
        int pos2 = str.indexOf(']');
        return str.mid(pos + 1, pos2 - pos - 1);
    }

    const QString ui_net_getCurrentInterfaceHumanNamme(void){
        int ns = this->ui->cbbNetworkInterface->count();
        if(ns == 0) return QString("");
        const QString str = this->ui->cbbNetworkInterface->currentText();
        int pos = str.indexOf('[');

        return str.mid(0, pos - 1);
    }

    const QString ui_net_getCurrentInterfaceAddr(bool ipv6 = false){
        int ns = this->ui->cbbNetworkInterface->count();
        if(ns == 0) return QString("");

        const QString str = ui_net_getCurrentInterfaceHardAddr();

        QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
        QNetworkInterface interface;
        QHostAddress addr;
        QList<QNetworkAddressEntry> alist;

        if(list.isEmpty()) return QString("");
        for(int i = 0; i < list.size(); i++){
            interface = list.at(i);
            if(str == interface.hardwareAddress()){
                alist = interface.addressEntries();
                if(alist.isEmpty()) return QString("");
                for(int j = 0; j < alist.size(); j++){
                    addr = alist.at(j).ip();
                    if(addr.isLoopback()){
                        goto ip_section;
                    }
                    if(!addr.isGlobal() || addr.isNull()) continue;
                ip_section:
                    uint32_t ip = addr.toIPv4Address();
                    if(ipv6){
                        return QHostAddress(addr.toIPv6Address()).toString();
                    }
                    if(ip) return QHostAddress(ip).toString();
                }
            }
        }
        return QString("");
    }

    void ui_net_setCurrentInterface(QNetworkInterface interface){
        const QString str = ui_net_makeInterfaceStr(interface);
        int find = this->ui->cbbNetworkInterface->findText(str);

        if(find != -1){
            this->ui->cbbNetworkInterface->setCurrentIndex(find);
        }else{
            this->ui->cbbNetworkInterface->addItem(str);
            this->ui->cbbNetworkInterface->setCurrentIndex(this->ui->cbbNetworkInterface->count() - 1);
        }
    }

    QNetworkInterface ui_net_getInterface(bool *ok){
        int ns = this->ui->cbbNetworkInterface->count();
        *ok = false;
        if(ns == 0){
            return QNetworkInterface();
        }
        QString str = ui_net_getCurrentInterfaceHardAddr();

        QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
        QNetworkInterface interface;
        QHostAddress addr;
        QList<QNetworkAddressEntry> alist;
        if(list.isEmpty()){
            return QNetworkInterface();
        }
        for(int i = 0; i < list.size(); i++){
            interface = list.at(i);
            if(str == interface.hardwareAddress()){
                *ok = true;
                return interface;
            }
        }
        return QNetworkInterface();
    }

    bool ui_net_isEnableIPV6(void){
        return this->ui->cbEnableIPV6->isChecked();
    }

    int ui_net_getRole(void){
        return this->ui->cbbNetRole->currentIndex();
    }

    int ui_net_getProfile(void){
        return this->ui->cbbNetProfile->currentIndex();
    }

    int ui_net_getPort(void){
        return this->ui->sbSourcePort->value();
    }

    const QString ui_net_getIP(void){
        return this->ui->leIPSource->text();
    }

    void ui_net_setIP(const QString &ip){
        this->ui->leIPSource->setText(ip);
    }


    // Tabel View

    void ui_net_addConnectionToTabel(const QString& ip, int id){
        this->ui->tbvClientConnection;
    }

    void ui_showTime();
    void ui_showSend(const QString &str, bool t = false);
    void ui_showRecieve(const QString &str, bool t = false);
    void ui_clearRecieve(void);
    void ui_showRecieveData(const QByteArray& data, int len);
//    void ui_showSendData(const QByteArray& data, int len);

    void ui_addSendHistory(const QString &str);
    void ui_addSendHistory(const QStringList& list);
    void ui_clearSendHistory(void);

    void ui_recieve_initRecieveFontColor(void){
//        QList<QColor> colors = {Qt::black, Qt::blue, Qt::darkGray,  Qt::white};
        QImage im(QSize(64, 32), QImage::Format_RGB32);
//        int s = colors.size();
        for(int i = 2; i < 19; i++){
            im.fill(Qt::GlobalColor(i));
            this->ui->cbbRecFontColor->addItem(QIcon(QPixmap::fromImage(im)), "");
        }
        this->ui->cbbRecFontColor->setCurrentIndex(7);
    }
    QColor ui_recieve_getRecieveFontColor(void){
        return Qt::GlobalColor(this->ui->cbbRecFontColor->currentIndex() + 2);
    }
    void ui_recieve_setRecieveFontColorState(bool state){
        this->ui->cbRecShowFontColor->setChecked(state);
    }

    void ui_setShowPlaintFont(const QFont &font){
        this->ui->pteSend->setFont(font);
        this->ui->pteShowRecieve->setFont(font);
    }
public:
    // Serial
    const int ui_serial_getPortNumber(void){ return this->ui->cbbPort->count(); }
    const QString ui_serial_getPortName(void){
        if(ui_serial_getPortNumber() == 0) return QString::fromUtf8("");
        const QString pn = this->ui->cbbPort->currentText();
        int pos = pn.indexOf(']');
        return pn.mid(1, pos - 1);
    }
    qint64 ui_serial_getBaud(void){ return this->ui->cbbBaud->currentText().toLongLong(); }
    QSerialPort::DataBits ui_serial_getDataBit(void){ return QSerialPort::DataBits(this->ui->cbbDataBit->currentText().toInt()); }
    QSerialPort::Parity ui_serial_getParity(void){
        int pi = this->ui->cbbParity->currentIndex();
        if(pi > 0){
            pi += 1;
        }
        return QSerialPort::Parity(pi);
    }
    QSerialPort::StopBits ui_serial_getStopBit(void){ return QSerialPort::StopBits(this->ui->cbbStop->currentIndex() + 1); }
    QSerialPort::FlowControl ui_serial_getFlow(void){ return QSerialPort::FlowControl(this->ui->cbbFlow->currentIndex()); }

    // Recieve
    int ui_recieve_getRecieveMode(void){
        if(this->ui->rbtRASCII->isChecked()){
            return ASCII_MODE;
        }
        if(this->ui->rbtRHex->isChecked()){
            return HEX_MODE;
        }
        return ASCII_MODE;
    }

    bool ui_show_isEnableAutoNewLine(void){
        return this->ui->cbAutoNewLine->isChecked();
    }

    bool ui_show_isEnableShowSend(void){
        return this->ui->cbShowSend->isChecked();
    }

    bool ui_show_isEnableShowTime(void){
        return this->ui->cbShowTime->isChecked();
    }
    bool ui_isEnableBufferMode(void){
        return this->ui->cbRecBufferMode->isChecked();
    }
    bool ui_show_isEnableShowColor(void){
        return this->ui->cbRecShowFontColor->isChecked();
    }
    int ui_recvieve_getBufferSize(void){
        return this->ui->sbRecBufferSize->value();
    }

    // Send
    int ui_send_getSendMode(void){
        if(this->ui->rbtSASCII->isChecked()){
            return ASCII_MODE;
        }
        if(this->ui->rbtSHex->isChecked()){
            return HEX_MODE;
        }
        return ASCII_MODE;
    }

    bool ui_send_isEnableAutoRepeat(void){
        return this->ui->cbAutoResend->isChecked();
    }
    void ui_send_setAutoRepeatState(bool set){
        this->ui->cbAutoResend->setCheckState(set ? Qt::Checked : Qt::Unchecked);
    }
    int ui_send_getRepeatTime(void){
        return this->ui->sbRetime->value();
    }

    int ui_send_getRepeatTimeUnit(void){
        return this->ui->cbbRetimeUnit->currentIndex();
    }

    // LOG
    const QString ui_log_getLogPath(){
        return this->ui->leLogPath->text();
    }
    bool ui_log_isEnableLog(void){
        return this->ui->cbEnableLog->isChecked();
    }

protected:
    bool event(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *event) override;
    void enterEvent(QEvent *event) override;
    void actionEvent(QActionEvent *event) override;
    // Status Bar

//    void ui_showMessage(const QString & m);
};
#endif // MAINWINDOW_H
