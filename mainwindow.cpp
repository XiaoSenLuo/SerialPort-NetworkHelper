#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    const QMutexLocker locker(&m_mutex);


    ui_creatToolBar();
    ui_creatStatusBar();

    ui_serial_toggle_pbtSend(serialStatus ? false : true);

    refresh_port_timer.start(500);
//    qDebug() << "MainWindow:" << QThread::currentThread();

    ui_recieve_initRecieveFontColor();

    ui_initSetting(&settingConfig);

    ui_statusbar_showRxBytes(rxBytes);
    ui_statusbar_showTxBytes(txBytes);

    s_helper = new SerialHelper(this);

    if(ui_getCurrentTab() == TAB_NETWORK){
        ui_net_initStatusTable();
    }

    ui_initConnect();
}

MainWindow::~MainWindow()
{
    delete ui;
    if(s_helper->isOpen()){
        s_helper->close();
        s_helper->deleteLater();
    }
}

const QString MainWindow::ui_getSendData()
{
    return this->ui->pteSend->toPlainText();
}

void MainWindow::ui_setSendData(const QString &str)
{
    this->ui->pteSend->clear();
    this->ui->pteSend->insertPlainText(str);
    this->ui->pteSend->moveCursor(QTextCursor::EndOfLine);
}

void MainWindow::ui_clearSendData()
{
    this->ui->pteSend->clear();
    this->ui->pteSend->moveCursor(QTextCursor::Start);
}

void MainWindow::serial_send(const QString &data, int len)
{
//    qDebug() << "MainWindow::ui_serial_send:" << QThread::currentThread();
    QString sendStr = QString(data);

    sendStr.replace(QLatin1String("\n"), QLatin1String(settingConfig.lineEnd[settingConfig.lineMode]));

    if(settingConfig.sendConfig.sendMode == ASCII_MODE){
        ui_addSendHistory(sendStr);
        const QByteArray sendBytes = sendStr.toUtf8();
        len = sendBytes.size();

        if(ui_send_isEnableAutoRepeat()) s_helper->setAutoWrite(sendBytes);
        else s_helper->write(sendBytes);

        emit ui_serial_send(sendBytes, len);

        txBytes += len;
    }
    if(settingConfig.sendConfig.sendMode == HEX_MODE){
        const QByteArray hex = sendStr.toUtf8().toHex().toUpper();
        len = hex.size();

        s_helper->write(hex);

        if(ui_send_isEnableAutoRepeat()) s_helper->setAutoWrite(hex);


        emit ui_serial_send(hex, len);

        sendStr = QString::fromUtf8(hex);
        ui_addSendHistory(sendStr);
        txBytes += len;
    }
    if(settingConfig.showConfig.enableShowSend){
        ui_showSend(sendStr, settingConfig.showConfig.enableShowTime);
    }else{
        if(settingConfig.logConfig.enableSaveLog){
            if(settingConfig.sendConfig.sendMode == ASCII_MODE){
                sendStr.append("\n");
            }
            ui_log_logSaveToFile(sendStr);
        }
    }
    ui_statusbar_showTxBytes(txBytes);
}

void MainWindow::serial_send(void)
{
    QString sendStr = ui_getSendData();
    int len = sendStr.size();

    serial_send(sendStr, len);
}

void MainWindow::ui_start_resend()
{
    int n = this->ui->sbRetime->value();
#ifdef SERIAL_THREAD
    if(resend_timer.isActive()) resend_timer.stop();
#endif
    if(n == 0){
        return;
    }
    int index = this->ui->cbbRetimeUnit->currentIndex();
    switch(index){
    case 0:
        n *= 1;
        break;
    case 1:
        n *= 1000;
        break;
    case 2:
        n *= (1000 * 60);
        break;
    case 3:
        n *= (1000 * 60 * 60);
        break;
    }
#ifdef SERIAL_THREAD
    resend_timer.start(n);
#else
    s_helper->setAutoWritePriod(n);
    s_helper->startAutoWrite();
#endif

}

void MainWindow::ui_showMessage(const QString &message, int time, QColor color)
{
    //    this->ui->statusbar->showMessage(message, time);
    //    if(color.isValid()) ui->statusbar->setStyleSheet(QString::fromUtf8("QStatusBar{color:%1;}\n").arg(color.name(QColor::HexArgb)));
    ui_statusbar_showMessage(message, color);
}


void MainWindow::handle_serial_recieve_data(const QByteArray &data, int len)
{
    rxBytes += len;
    ui_statusbar_showRxBytes(rxBytes);
    if(settingConfig.showConfig.enableAutoNewLine){
//        int ep = data.indexOf(QByteArray(settingConfig.lineEnd[settingConfig.lineMode]));
//        if(ep >= 0){

//        }
        ui_showRecieveData(data, len);
    }else{
        ui_showRecieveData(data, len);
    }
}


void MainWindow::handle_serial_error(int err)
{
    QSerialPort::SerialPortError error = (QSerialPort::SerialPortError)err;
    SerialConfig config = ui_serial_getConfig();
    qDebug() << "Serial Error:" << config.errorStr[int(error)];
    switch(error){
    case QSerialPort::NoError:
        if(serialStatus == STATUS_OPEN){
            ui_serial_toggle_pbtSend(true);
            const QString mes = QString::fromUtf8("%1 OPENED [%2][%3][%4][%5][%6]").arg(config.portName).arg(config.portBaud).arg(config.portDataBit).arg(config.parityStr[config.portParity]).arg(config.stopStr[config.portStopBit]).arg(config.flowStr[config.portFlow]);
            ui_showMessage(mes);
        }
        if(serialStatus == STATUS_CLOSE){
            if(ui_serial_getPortNumber()){
                const QString mes = QString::fromUtf8("%1 CLOSE").arg(config.portName);
                ui_showMessage(mes, 0, Qt::red);
            }else{
                ui_showMessage("");
            }
        }
        break;
    case QSerialPort::TimeoutError:
        break;
    case QSerialPort::NotOpenError:
        if(serialStatus == STATUS_CLOSE) break;
    default:
        ui_serial_toggle_pbtSend(false);
        const QString mes = QString::fromUtf8("%1 串口发生意外错误 [%2]").arg(ui_serial_getPortName()).arg(config.errorStr[(int)error]);
        ui_showMessage(mes, 0, Qt::black);

        serialStatus = STATUS_OCCUR_ERROR;
        break;
    }
}

void MainWindow::handle_serialhelper_readyread()
{
    qint64 len = 0, tmp = 0;
    QByteArray data;

    tmp = s_helper->bytesAvailable();
    while(tmp > 0){
        len += tmp;
        data.append(s_helper->readAll());
        tmp = s_helper->bytesAvailable();
    }
    if(serialStatus == STATUS_OPEN) handle_serial_recieve_data(data, len);
}


void MainWindow::handle_ui_line_mode_changed(int index)
{
    settingConfig.lineMode = index;
}

void MainWindow::handle_ui_line_mode_changed(const QString &perf)
{
    if(perf.contains("win", Qt::CaseInsensitive)){
        settingConfig.lineMode = WIN_MODE;
        return;
    }
    if(perf.contains("unix", Qt::CaseInsensitive)){
        settingConfig.lineMode = UNIX_MODE;
        return;
    }
    if(perf.contains("macos", Qt::CaseInsensitive) && !perf.contains("legacy", Qt::CaseInsensitive)){
        settingConfig.lineMode = MACOS_MODE;
        return;
    }
    if(perf.contains("macos", Qt::CaseInsensitive) && perf.contains("legacy", Qt::CaseInsensitive)){
        settingConfig.lineMode = MACOS_LEGACY_MODE;
        return;
    }
}

void MainWindow::update_ui_serial_config()
{
//    qDebug() << "MainWindow::handle_serial_config:" << QThread::currentThread();
    const SerialConfig config = ui_serial_getConfig();

    settingConfig.serialConfig = config;

#ifdef SERIAL_THREAD
    m_mutex.lock();
    serialConfig = config;
    m_mutex.unlock();

    if((serialStatus == STATUS_OPEN) || (serialStatus == STATUS_PAUSE)){
//        qDebug() << "| Port:" << config.portName << " | Baud:" << config.portBaud << " | DataBit:" << config.portDataBit << " | Parity:" << config.portParity << " | StopBit:" << config.portStopBit << " | Flow:" << config.portFlow;
        emit ui_serial_config_changed();
    }
#endif

}

void MainWindow::ui_statusbar_showMessage(const QString &str, QColor color)
{
    this->slabel->setText(str);
    this->slabel->setStyleSheet(QString::fromUtf8("QLabel{color:%1;font:%2pt \"%3\";}").arg(color.name(QColor::HexRgb)).arg(this->font().pointSize()).arg(this->font().family()));
}

void MainWindow::ui_statusbar_showRxBytes(ulong bytes)
{
    this->lbRxBytes->setText(QString::fromUtf8("%1 Bytes").arg(bytes));
}

void MainWindow::ui_statusbar_showTxBytes(ulong bytes)
{
    this->lbTxBytes->setText(QString::fromUtf8("%1 Bytes").arg(bytes));
}

void MainWindow::ui_statusbar_showLogPath(const QString &str)
{
    this->lbLogPath->setText(str);
    this->lbLogPath->setToolTip(str);
}

int MainWindow::ui_getCurrentTab()
{
    return this->ui->tabWidget->currentIndex();
}

void MainWindow::ui_log_setLogPath(const QString &p)
{
    ui_statusbar_showLogPath(p);
    this->ui->leLogPath->setText(p);
    this->ui->leLogPath->setToolTip(p);
    this->ui->leLogPath->setWhatsThis(QString::fromUtf8("日志文件路径"));
}

void MainWindow::ui_log_setSaveLogState(bool save)
{
    this->ui->cbEnableLog->setCheckable(save);
}

void MainWindow::handle_setting_changed(SettingConfig config)
{
    settingConfig = config;

    ui_log_setLogPath(settingConfig.logConfig.filePath);
    ui_log_setSaveLogState(settingConfig.logConfig.enableSaveLog);

    if(settingConfig.logConfig.enableBuffer) fileBuffer.resize(settingConfig.logConfig.bufferSize * 1024);

    ui_recieve_setRecieveFontColorState(settingConfig.showConfig.enableShowColor);

    ui_setShowPlaintFont(settingConfig.showConfig.font);
}

void MainWindow::ui_initSetting(SettingConfig* config)
{
    if(!config) return ;
    config->logConfig.enableBuffer = ui_log_isEnableLog();
    const QString log_path = QApplication::applicationDirPath();
    ui_log_setLogPath(log_path);
    settingConfig.logConfig.filePath = log_path;

    config->recConfig.bufferMode = ui_isEnableBufferMode();
    config->recConfig.showMode = ui_recieve_getRecieveMode();
    config->recConfig.bufferSize = ui_recvieve_getBufferSize();

    config->sendConfig.ResendTime = ui_send_getRepeatTime();
    config->sendConfig.ResendUnit = ui_send_getRepeatTimeUnit();
    config->sendConfig.enableAutoResend = ui_send_isEnableAutoRepeat();
    config->sendConfig.sendMode = ui_send_getSendMode();

    config->showConfig.enableAutoNewLine = ui_show_isEnableAutoNewLine();
    config->showConfig.enableShowColor = ui_show_isEnableShowColor();
    config->showConfig.enableShowSend = ui_show_isEnableShowSend();
    config->showConfig.enableShowTime = ui_show_isEnableShowTime();
    config->showConfig.recColor = ui_recieve_getRecieveFontColor();
    settingConfig.showConfig.font = this->font();

    config->netConfig.ip = ui_net_getIP();
    config->netConfig.port = ui_net_getPort();
    config->netConfig.netRole = ui_net_getRole();
    config->netConfig.netProfile = ui_net_getProfile();
}

void MainWindow::ui_initConnect()
{
    QObject::connect(s_helper, &SerialHelper::errorOccurred, this, &MainWindow::handle_serial_error);
    QMetaObject::Connection s_connect = s_helper->callOnReadyRead(this, &MainWindow::handle_serialhelper_readyread);

    QObject::connect(&refresh_port_timer, &QTimer::timeout, [=](){
        isAutorefresh = true;
        if(ui_getCurrentTab() == 0){   // 串口
//           ui_refreshPort();
           int pc = ui->cbbPort->count();
           QList<QSerialPortInfo> qList = QSerialPortInfo::availablePorts();
           int sc = qList.count();
           int ci = this->ui->cbbPort->currentIndex();
           const QString cpn = ui_serial_getPortName();
           if(pc != sc){
               int inser = 0;
               int find = -1;
               this->ui->cbbPort->clear();
               for(int i = 0; i < sc; i++){
                   if(cpn == qList.at(i).portName()){
                       find = i;
                       if(find > ci){
                           this->ui->cbbPort->insertItem(ci, ui_serial_makePortName(qList.at(find)));
                           continue;
                       }
                       if(find < ci){
                           inser = (ci < sc) ? (ci - find) : (sc - find);
                       }
                   }
                   if(inser != 0){
                       inser -= 1;
                       this->ui->cbbPort->insertItem(find, ui_serial_makePortName(qList.at(i)));
                       continue;
                   }
                   ui_serial_addPortName(qList.at(i));
               }
               isAutorefresh = false;
               if( ci > sc - 1) ci = sc - 1;
               this->ui->cbbPort->setCurrentIndex((ci < 0) ? 0 : ci);
//               return;
           }
        }
        if(ui_getCurrentTab() == 1){   // TCP/UDP
//            ui_refreshNetInterface();

            QList<QNetworkInterface> nlist = QNetworkInterface::allInterfaces();
            int ls = nlist.size();
            if(ls == 0) return;
            int ns = this->ui->cbbNetworkInterface->count();

            if(ls != ns){
                int ci = this->ui->cbbNetworkInterface->currentIndex();
                int inser = 0;
                int find = -1;
                QHostAddress addr;
                QNetworkInterface interface;
                int eth = -1;
                const QString chaddr = ui_net_getCurrentInterfaceHardAddr();
                this->ui->cbbNetworkInterface->clear();
                for(int i = 0; i < ls; i++){
                    interface = nlist.at(i);
                    if(ns && (chaddr == interface.hardwareAddress())){
                        find = i;
                        if(find > ci){
                            this->ui->cbbNetworkInterface->insertItem(ci, ui_net_makeInterfaceStr(interface));
                            continue;
                        }
                        if(find < ci){
                            inser = (ci < ls) ? (ci - find) : (ls - find);
                        }
                    }
                    if(inser != 0){
                        inser -= 1;
                        this->ui->cbbNetworkInterface->insertItem(find, ui_net_makeInterfaceStr(interface));
                        continue;
                    }
                    ui_net_addInterface(interface);
                    if((eth < 0) && interface.type() == QNetworkInterface::Ethernet){
                        const QString nstr = interface.humanReadableName();
                        if(nstr.contains(QString::fromUtf8("以太网"))
                                || nstr.contains(QString::fromUtf8("有线"))){
                            eth  = i;
                        }
                    }
                }
                isAutorefresh = false;
                if(ci > ls - 1) ci = ls - 1;

                if(eth < 0) this->ui->cbbNetworkInterface->setCurrentIndex((ci < 0) ? 0 : ci);
                else{
                    this->ui->cbbNetworkInterface->setCurrentIndex(eth);
                }
//                return;
            }
        }
        isAutorefresh = false;

    });
    QObject::connect(this->ui->tabWidget, &QTabWidget::currentChanged, [=](int index){
        currentTab = index;
        if(index == 0){   // serial
            if(!s_helper) s_helper = new SerialHelper(this);
        }
        if(index == 1){   // tcp/udp

        }
    });
    QObject::connect(this->ui->cbbPort, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        if(isAutorefresh) return;   // 不响应自动刷新导致的变化
        if(index < 0) return;
        this->ui->cbbPort->setToolTip(this->ui->cbbPort->currentText());

        bool op = s_helper->isOpen();
        if(op) s_helper->close();
        s_helper->setPortName(ui_serial_getPortName());
        if(op) s_helper->open(QIODevice::ReadWrite);

        update_ui_serial_config();
        qDebug() << "ui:" << ui_serial_getPortName();
    });
    QObject::connect(this->ui->cbbBaud, QOverload<const QString &>::of(&QComboBox::currentTextChanged), [=](const QString &s){
        s_helper->setBaudRate(ui_serial_getBaud());
        update_ui_serial_config();
        qDebug() << "ui:" << s;
    });
    QObject::connect(this->ui->cbbDataBit, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        s_helper->setDataBits(ui_serial_getDataBit());
        update_ui_serial_config();
        qDebug() << "ui:" << ui_serial_getDataBit();
    });
    QObject::connect(this->ui->cbbParity, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        s_helper->setParity(ui_serial_getParity());
        update_ui_serial_config();
        qDebug() << "ui:" << ui_serial_getParity();
    });
    QObject::connect(this->ui->cbbStop, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        s_helper->setStopBits(ui_serial_getStopBit());
        update_ui_serial_config();
        qDebug() << "ui:" << ui_serial_getParity();
    });
    QObject::connect(this->ui->cbbFlow, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        s_helper->setFlowControl(ui_serial_getFlow());
        update_ui_serial_config();
        qDebug() << "ui:" << ui_serial_getParity();

    });
    QObject::connect(this->ui->actionStart, &QAction::triggered, [=](){      // 开始传输, 暂停之后可以再次开始
//        qDebug() << "actionStart:" << QThread::currentThread();
        if(ui_getCurrentTab() == MainWindow::TAB_SERIAL){
            if(ui_serial_getPortNumber() == 0) return;
            int tmp = serialStatus;
            if((tmp == STATUS_CLOSE) || (tmp == STATUS_OCCUR_ERROR)){
                serialStatus = STATUS_OPEN;
                const SerialConfig config = ui_serial_getConfig();
                apply_ui_serial_config(config);
                bool os = false;
                if(!s_helper->isOpen()) os = s_helper->open(QIODevice::ReadWrite);
                if(os) ui_serial_toggle_pbtSend(true);
                update_ui_serial_config();
#ifdef SERIAL_THREAD
                emit ui_serial_open();
#endif
            }
            if(tmp == STATUS_PAUSE){
                serialStatus = STATUS_OPEN;
#ifdef SERIAL_THREAD
                emit ui_serial_start();
#endif
                if(ui_send_isEnableAutoRepeat()) ui_start_resend();   // 自动重发
            }
        }
        if(ui_getCurrentTab() == MainWindow::TAB_NETWORK){

            int tmp = networkStatus;
            if(networkStatus == STATUS_CLOSE){
                networkStatus = STATUS_OPEN;
                settingConfig.netConfig.ip = ui_net_getIP();
                settingConfig.netConfig.netRole = ui_net_getRole();
                settingConfig.netConfig.port = ui_net_getPort();
                settingConfig.netConfig.netProfile = ui_net_getProfile();

                if(settingConfig.netConfig.netProfile == NetWorkSettingConfig::TCP){
                    if(!tcp_helper) tcp_helper = new TCPHelper(settingConfig.netConfig.netRole, this);
                    tcp_helper->start(QHostAddress(settingConfig.netConfig.ip), settingConfig.netConfig.port);
#ifdef NETWORK_THREAD
                    emit ui_tcp_start(settingConfig.netConfig.ip, settingConfig.netConfig.port, settingConfig.netConfig.netRole);
#endif
                }else{
                    if(tcp_helper){
                        tcp_helper->stop();
                        tcp_helper->deleteLater();
                        tcp_helper = nullptr;
                    }
                }
                if(settingConfig.netConfig.netProfile == NetWorkSettingConfig::UDP){
                    if(!udp_helper) udp_helper = new UDPHelper(settingConfig.netConfig.netRole, this);
                    udp_helper->start(QHostAddress(settingConfig.netConfig.ip), settingConfig.netConfig.port);
                }else{
                    if(udp_helper){
                        udp_helper->stop();
                        udp_helper->deleteLater();
                        udp_helper = nullptr;
                    }
                }
            }
        }
    });
    QObject::connect(this->ui->actionStop, &QAction::triggered, [=](){       // 停止传输, 此操作会关闭串口
        if(ui_getCurrentTab() == MainWindow::TAB_SERIAL){
            serialStatus = STATUS_CLOSE;
            ui_serial_toggle_pbtSend(false);
            if(ui_send_isEnableAutoRepeat()){
#ifdef SERIAL_THREAD
                resend_timer.stop();
#else
                s_helper->stopAutoWrite();
#endif
                ui_send_setAutoRepeatState(false);
            }
#ifdef SERIAL_THREAD
            emit ui_serial_close();
#else
            s_helper->close();
            const QString mes = QString::fromUtf8("%1 CLOSE").arg(ui_serial_getPortName());
            ui_showMessage(mes, 0, Qt::red);
#endif
        }
        if(ui_getCurrentTab() == MainWindow::TAB_NETWORK){
            if(ui_net_getProfile() == NetWorkSettingConfig::TCP){
#ifdef NETWORK_THREAD
                emit ui_tcp_stop(ui_net_getRole());
#else
                if(tcp_helper){
                    if(ui_send_isEnableAutoRepeat()){
                        tcp_helper->stopAutoWrite();
                    }
                    tcp_helper->stop();
                }
#endif
            }
            if(settingConfig.netConfig.netRole == NetWorkSettingConfig::UDP){
                if(udp_helper){
                    if(ui_send_isEnableAutoRepeat()){
                        udp_helper->stopAutoWrite();
                    }
                    udp_helper->stop();
                }
            }
        }
    });
    QObject::connect(this->ui->actionPause, &QAction::triggered, [=](){     // 暂停传输, 不会关闭串口
        if(ui_getCurrentTab() == MainWindow::TAB_SERIAL){
            if(serialStatus != STATUS_OPEN) return;
            serialStatus = STATUS_PAUSE;
            if(ui_send_isEnableAutoRepeat()){
#ifdef SERIAL_THREAD
                resend_timer.stop();
#else
                s_helper->stopAutoWrite();
#endif
            }
            emit ui_serial_pause();
        }
        if(ui_getCurrentTab() == MainWindow::TAB_NETWORK){

        }
    });
    QObject::connect(this->ui->actionClear, &QAction::triggered, [=](){        // 清空接收
        ui_clearRecieve();
    });
    QObject::connect(this->ui->actionClearSendHistory, &QAction::triggered, [=](){   // 清空发送历史
        ui_clearSendHistory();
    });

    // 切换行模式
    QActionGroup *lineActionGroup = new QActionGroup(this);
    lineActionGroup->addAction(this->ui->actionWin);
    lineActionGroup->addAction(this->ui->actionUnix);
    lineActionGroup->addAction(this->ui->actionMacOS);
    lineActionGroup->addAction(this->ui->actionMacOSLegacy);
    QObject::connect(lineActionGroup, &QActionGroup::triggered, [=](QAction *action){
        const QString objn = action->objectName();
        handle_ui_line_mode_changed(objn);
    });

    // 打开设置面板
    QObject::connect(this->ui->actionSetting, &QAction::triggered, [=](){
          SettingDialog settingDialog(settingConfig, this);
          QObject::connect(&settingDialog, &SettingDialog::setting_changed, this, &MainWindow::handle_setting_changed);
          int res = settingDialog.exec();
          if(res == QDialog::Accepted){

          }
          if(res == QDialog::Rejected){

          }
          QObject::disconnect(&settingDialog, &SettingDialog::setting_changed, this, &MainWindow::handle_setting_changed);
    });

    QObject::connect(this->ui->pbtSend, &QPushButton::clicked, [=](){   // 发送数据
//        qDebug() << "pbtSend:" << QThread::currentThread();
        if(ui_getCurrentTab() == 0){
            if(serialStatus == STATUS_OPEN){  // 发送数据
            sendData_section:
                serial_send();
                return;
            }
            if(serialStatus == STATUS_CLOSE){   // 先打开再发送数据
                SerialConfig config;
                config.portBaud = ui_serial_getBaud();
                config.portDataBit = ui_serial_getDataBit();
                config.portFlow = ui_serial_getFlow();
                config.portName = ui_serial_getPortName();
                config.portParity = ui_serial_getParity();
                config.portStopBit = ui_serial_getStopBit();

                settingConfig.serialConfig = config;
#ifdef SERIAL_THREAD
                m_mutex.lock();
                serialConfig = config;
                m_mutex.unlock();
#else
                apply_ui_serial_config(config);
                s_helper->open(QIODevice::ReadWrite);
#endif
                serialStatus = STATUS_OPEN;
                ui_serial_toggle_pbtSend(true);

                emit ui_serial_open();

                goto sendData_section;
            }
        }

    });
    QObject::connect(this->ui->pbtClearSend, &QPushButton::clicked, [=](){          // 清空发送输入文本框
        ui_clearSendData();
    });

    QObject::connect(this->ui->cbbSendHistory, QOverload<const QString&>::of(&QComboBox::textActivated), [=](const QString &str){  // 点击发送历史文本时将其输入到文本框
        ui_setSendData(str);
    });

    QObject::connect(this->ui->actionOpenLogDir, &QAction::triggered, [=](bool b){   // 选择日志目录
        QString _p = ui_log_getLogPath();
        int pos = _p.lastIndexOf('/');
        QString _pp = _p.mid(pos + 1);
        if(_pp.contains(".txt")){
            _p = _p.mid(0, pos);
        }
        const QString p = QFileDialog::getExistingDirectory(this, QString::fromUtf8("选择日志目录"), _p, QFileDialog::ShowDirsOnly);

        if(p.isEmpty()) return;
        ui_log_setLogPath(p);
    });

    QObject::connect(this->ui->rbtRASCII, &QRadioButton::toggled, [=](bool state){
        settingConfig.recConfig.showMode = ui_recieve_getRecieveMode();
    });

    QObject::connect(this->ui->rbtSASCII, &QRadioButton::toggled, [=](bool state){
        settingConfig.sendConfig.sendMode = ui_send_getSendMode();
    });

    QObject::connect(this->ui->cbAutoNewLine, &QCheckBox::stateChanged, [=](int state){
        if(state == Qt::Checked){
             settingConfig.showConfig.enableAutoNewLine = true;
        }
        if(state == Qt::Unchecked){
             settingConfig.showConfig.enableAutoNewLine = false;
        }
    });
    QObject::connect(this->ui->cbShowSend, &QCheckBox::stateChanged, [=](int state){
        if(state == Qt::Checked){
             settingConfig.showConfig.enableShowSend = true;
        }
        if(state == Qt::Unchecked){
             settingConfig.showConfig.enableShowSend = false;
        }
    });
    QObject::connect(this->ui->cbShowTime, &QCheckBox::stateChanged, [=](int state){
        if(state == Qt::Checked){
             settingConfig.showConfig.enableShowTime = true;
        }
        if(state == Qt::Unchecked){
             settingConfig.showConfig.enableShowTime = false;
        }
    });
#ifdef SERIAL_THREAD
    QObject::connect(&resend_timer, &QTimer::timeout, [=](){       // 定时发送
        if(ui_getCurrentTab() == 0){
            if(serialStatus == STATUS_OPEN){
                serial_send();
            }
        }
    });
#endif
    QObject::connect(this->ui->cbAutoResend, &QCheckBox::stateChanged, [=](int state){    // 是否定时发送
        if(state == Qt::Checked){
            settingConfig.sendConfig.enableAutoResend = true;
            if(serialStatus != STATUS_OPEN) return;
#ifdef SERIAL_THREAD
#else
            s_helper->setAutoWrite(ui_getSendData().toUtf8());
#endif
            ui_start_resend();
        }
        if(state == Qt::Unchecked){
            settingConfig.sendConfig.enableAutoResend = false;
#ifdef SERIAL_THREAD
            if(resend_timer.isActive()) resend_timer.stop();
#else
            s_helper->stopAutoWrite();
#endif
        }
    });

    QObject::connect(this->ui->sbRetime, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value){    // 设置定时发送时间
        settingConfig.sendConfig.ResendTime = value;
        if(serialStatus != STATUS_OPEN) return;
        if(settingConfig.sendConfig.enableAutoResend) ui_start_resend();
    });
    QObject::connect(this->ui->cbbRetimeUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){    // 设置定时时间单位
        settingConfig.sendConfig.ResendUnit = index;
        if(serialStatus != STATUS_OPEN) return;
        if(settingConfig.sendConfig.enableAutoResend) ui_start_resend();
    });
    QObject::connect(this->ui->tbtSeleteLogPath, &QToolButton::clicked, [=](bool b){  // 选择日志路径
        ui_log_seleteLogPath();
    });
    QObject::connect(this->ui->cbEnableLog, &QCheckBox::stateChanged, [=](int state){
        if(state == Qt::Checked){
            QString _lp = ui_log_getLogPath();
            int pos = _lp.lastIndexOf('/');
            QString _ln = _lp.mid(pos + 1);
            if(!_ln.contains(".txt")){  // 不含txt文件, 说明是目录
                 _lp.append(QString::fromUtf8("/log_%1.txt").arg(QDateTime::currentDateTime().toString(QString::fromUtf8("yyyyMMddhhmmss"))));
            }
            settingConfig.logConfig.filePath = _lp;
            ui_log_setLogPath(_lp);
            settingConfig.logConfig.enableSaveLog = true;
        }
        if(state == Qt::Unchecked){
            settingConfig.logConfig.enableSaveLog = false;
        }
    });

    QObject::connect(this->ui->cbbRecFontColor, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){   // 选择显示接收字体颜色
        QColor color = ui_recieve_getRecieveFontColor();
        settingConfig.showConfig.recColor = color;
    });
    QObject::connect(this->ui->cbRecShowFontColor, &QCheckBox::stateChanged, [=](int state){     // 是否显示颜色, 不使能将固定黑色显示
        if(state == Qt::Checked){
            settingConfig.showConfig.enableShowColor = true;
        }
        if(state == Qt::Unchecked){
            settingConfig.showConfig.enableShowColor = false;
        }
    });
    QObject::connect(this->ui->cbRecBufferMode, &QCheckBox::stateChanged, [=](int state){  // 是否使能接收缓冲模式, 有些串口接收长数据会固定分段接收, 此模式可以将分段数据缓存, 一起显示
        if(state == Qt::Checked){
            settingConfig.recConfig.bufferMode = true;
        }
        if(state == Qt::Unchecked){
            settingConfig.recConfig.bufferMode = false;
        }
    });
    QObject::connect(this->ui->sbRecBufferSize, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value){  // 接收缓存大小, 单位字节
        settingConfig.recConfig.bufferSize = value;
    });


    //网络
    QObject::connect(this->ui->cbbNetworkInterface, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){  // 网卡接口
        bool res = false;
        QNetworkInterface interface = ui_net_getInterface(&res);
        if(res) settingConfig.netConfig.interface = interface;

        if(ui_net_getRole() == NetWorkSettingConfig::Server){
            const QString ip = ui_net_getCurrentInterfaceAddr();
            ui_net_setIP(ip);
        }
    });

    QObject::connect(this->ui->cbbNetProfile, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){  // 网络协议
        if(index == 0){  // Client
            settingConfig.netConfig.netProfile = NetWorkSettingConfig::TCP;
        }
        if(index == 1){   // Server
            settingConfig.netConfig.netProfile = NetWorkSettingConfig::UDP;
        }
        ui_net_role_function(index);
    });
    QObject::connect(this->ui->cbbNetRole, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){  // 网络角色
        if(index == 0){  // Client
            settingConfig.netConfig.netRole = NetWorkSettingConfig::Client;
        }
        if(index == 1){   // Server
            settingConfig.netConfig.netRole = NetWorkSettingConfig::Server;

            const QString ip = ui_net_getCurrentInterfaceAddr();
            this->ui->leIPSource->setText(ip);
//            this->ui->leIPSource->setEnabled(false);
        }
        if(index == 2){   // 多播
            settingConfig.netConfig.netRole = NetWorkSettingConfig::Multicast;
        }
        if(index == 3){   // 广播
            settingConfig.netConfig.netRole = NetWorkSettingConfig::Broadcast;
            this->ui->leIPSource->setText(QHostAddress(QHostAddress::Broadcast).toString());
        }
    });

    QObject::connect(this->ui->sbSourcePort, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value){
        settingConfig.netConfig.port = value;
    });

    // 初始化TableWidget 信号槽连接
    // 单击, 单次选择
    QObject::connect(this->ui->tbwShowConnnectStatus, &QTableWidget::cellClicked, [=](int row, int column){
        qDebug() << QString::fromUtf8("单击选择: ") << "(" << row << ", " << column << ")";
        int minRow = 0, maxRow = 0;
//        Qt::KeyboardModifiers key = QApplication::keyboardModifiers();

        QList<QTableWidgetSelectionRange> ranges = this->ui->tbwShowConnnectStatus->selectedRanges();
        if(ranges.isEmpty()) return;

        for(int i = 0; i < ranges.size(); i++){
            int _max = std::max(ranges.at(i).topRow(), ranges.at(i).bottomRow());
            int _min = std::min(ranges.at(i).topRow(), ranges.at(i).bottomRow());
            int m = _min;
            int index = -1;
//            while (m <= _max) {
//                index = seleteRows.indexOf(m);
//                if(index < 0){
//                    seleteRows.append(m);
//                    m += 1;
//                }
//            }
//                qDebug() << "[" << i << "]:" << ranges.at(i).topRow() << "|" << ranges.at(i).leftColumn() << "|" << ranges.at(i).bottomRow() << "|" << ranges.at(i).rightColumn() ;
//                this->ui->tbwShowConnnectStatus->setRangeSelected(QTableWidgetSelectionRange(ranges.at(i).topRow(), 0, ranges.at(i).bottomRow(), this->ui->tbwShowConnnectStatus->columnCount() - 1), true);
        }

//        minRow = (minRow > row) ? row : minRow;
//        maxRow = (maxRow < row) ? row : maxRow;
//        qDebug() << minRow << "|" << maxRow;
//        this->ui->tbwShowConnnectStatus->setRangeSelected(QTableWidgetSelectionRange(minRow, 0, maxRow, this->ui->tbwShowConnnectStatus->columnCount() - 1), true);
    });
    // 双击, 移除选择
    QObject::connect(this->ui->tbwShowConnnectStatus, &QTableWidget::cellDoubleClicked, [=](int row, int column){
//        QList<QTableWidgetSelectionRange> ranges = this->ui->tbwShowConnnectStatus->selectedRanges();
        QList<QTableWidgetSelectionRange> ranges = this->ui->tbwShowConnnectStatus->selectedRanges();
        for(int i = 0; i < ranges.size(); i++){

            this->ui->tbwShowConnnectStatus->setRangeSelected(QTableWidgetSelectionRange(ranges.at(i).topRow(), 0, ranges.at(i).bottomRow(), this->ui->tbwShowConnnectStatus->columnCount() - 1), true);
        }
        this->ui->tbwShowConnnectStatus->setRangeSelected(QTableWidgetSelectionRange(row, 0, row, this->ui->tbwShowConnnectStatus->columnCount() - 1), false);

    });

    QObject::connect(this->ui->tbwShowConnnectStatus, &QTableWidget::customContextMenuRequested, [=](const QPoint &pos){
        qDebug() << pos;

    });
}

void MainWindow::ui_creatStatusBar()
{
//    MStatusBar *sbar = new MStatusBar(this->ui->statusbar);
//    sbar->setGeometry(this->ui->statusbar->geometry().x(), this->ui->statusbar->geometry().y(), this->ui->statusbar->geometry().width() * 0.7, this->ui->statusbar->geometry().height());

//    this->ui->statusbar->addPermanentWidget(sbar, 0);
//    sttklabel = new TTKMarqueeLabel(this->ui->statusbar);

    slabel = new QLabel(this->ui->statusbar);
    slabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    slabel->setFixedHeight(16);
    slabel->setFixedSize(256 + 64, 16);

    QLabel *label = new QLabel(QString("Rx: "), this->ui->statusbar);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel *label1 = new QLabel(QString("Tx: "), this->ui->statusbar);
    label1->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    lbTxBytes = new QLabel(this->ui->statusbar);
    lbTxBytes->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lbTxBytes->setFont(QFont(this->font()));
    lbTxBytes->setFixedSize(128, 16);

    lbRxBytes = new QLabel(this->ui->statusbar);
    lbRxBytes->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lbRxBytes->setFont(QFont(this->font()));
    lbRxBytes->setFixedSize(128, 16);

    lbLogPath = new TTKMarqueeLabel(this->ui->statusbar);
    lbLogPath->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    lbLogPath->setFont(this->font());
    lbLogPath->setMoveStyle(TTKMarqueeLabel::MoveStyleLeftToRight);
    lbLogPath->setMouseHoverStop(true);
    lbLogPath->setInterval(100);
    lbLogPath->setForeground(Qt::blue);
    lbLogPath->setFixedHeight(16);

    this->ui->statusbar->addPermanentWidget(slabel, 1);
    this->ui->statusbar->addPermanentWidget(label, 1);
    this->ui->statusbar->addPermanentWidget(lbRxBytes, 1);
    this->ui->statusbar->addPermanentWidget(label1, 1);
    this->ui->statusbar->addPermanentWidget(lbTxBytes, 1);
    this->ui->statusbar->addPermanentWidget(lbLogPath, 1);
}

void MainWindow::ui_creatToolBar()
{
    this->ui->toolBar->addAction(this->ui->actionStart);
    this->ui->toolBar->addAction(this->ui->actionPause);
    this->ui->toolBar->addAction(this->ui->actionStop);
    this->ui->toolBar->addAction(this->ui->actionClear);
    this->ui->toolBar->addSeparator();
    this->ui->toolBar->addAction(this->ui->actionSetting);
}

void MainWindow::ui_refreshPort()
{
    int pc = ui->cbbPort->count();
    QList<QSerialPortInfo> qList = QSerialPortInfo::availablePorts();
    int sc = qList.count();
    int ci = this->ui->cbbPort->currentIndex();
    const QString cpn = ui_serial_getPortName();
    if(pc != sc){
        int inser = 0;
        int find = -1;
        this->ui->cbbPort->clear();
        for(int i = 0; i < sc; i++){
            if(cpn == qList.at(i).portName()){
                find = i;
                if(find > ci){
                    this->ui->cbbPort->insertItem(ci, ui_serial_makePortName(qList.at(find)));
                    continue;
                }
                if(find < ci){
                    inser = (ci < sc) ? (ci - find) : (sc - find);
                }
            }
            if(inser != 0){
                inser -= 1;
                this->ui->cbbPort->insertItem(find, ui_serial_makePortName(qList.at(i)));
                continue;
            }
            ui_serial_addPortName(qList.at(i));
        }
        isAutorefresh = false;
        if( ci > sc - 1) ci = sc - 1;
        this->ui->cbbPort->setCurrentIndex((ci < 0) ? 0 : ci);
        return;
    }
}

void MainWindow::ui_refreshNetInterface()
{

}

void MainWindow::apply_ui_serial_config(const SerialConfig &config)
{
    // 串口可以动态改变的参数
//    if(config.portBaud != settingConfig.serialConfig.portBaud){
        s_helper->setBaudRate(config.portBaud);
//    }
//    if(config.portDataBit != settingConfig.serialConfig.portDataBit){
        s_helper->setDataBits(config.portDataBit);
//    }
//    if(config.portStopBit != settingConfig.serialConfig.portStopBit){
        s_helper->setStopBits(config.portStopBit);
//    }
        s_helper->setFlowControl(config.portFlow);
        s_helper->setParity(config.portParity);

//    if(settingConfig.serialConfig.portName.compare(config.portName) == 0){     // 端口发生改变, 串口的端口不可以动态改变
        bool op = s_helper->isOpen();
        QIODevice::OpenMode md = s_helper->openMode();
        if(op) s_helper->close();
        s_helper->setPortName(config.portName);
        if(op) s_helper->open(md);
//    }
}

void MainWindow::ui_log_logSaveToFile(const QString &str)
{
    if(!settingConfig.logConfig.enableSaveLog) return;
    if(settingConfig.logConfig.enableBuffer){   // 使能缓冲区

    }else{
        QFile file(settingConfig.logConfig.filePath);
        if(!file.open(QIODevice::ReadWrite | QIODevice::Append)){
            return;
        }
        QTextStream out(&file);
        out << str;
        file.close();
    }
}

void MainWindow::ui_log_seleteLogPath()
{
    QString _p = ui_log_getLogPath();
    if(_p.isEmpty()){
        _p = QApplication::applicationDirPath();
    }else{
        int pos = _p.lastIndexOf('/');
        QString _pp = _p.mid(pos + 1);
        if(_pp.contains(".txt")){
            _p = _p.mid(0, pos);
        }
    }
    QFileDialog dialog(this, QString::fromUtf8("保存日志文件"), _p, QString::fromUtf8("文本文件 (*.txt)"));
    QString dt_str = QDateTime::currentDateTime().toString(QString::fromUtf8("log_yyyyMMddhhmmss"));
    dt_str = dt_str + QString::fromUtf8(".txt");
    dialog.selectFile(dt_str);
    int res = dialog.exec();
    if(res == QFileDialog::Accepted){
        const QString p = dialog.selectedFiles().first();
        settingConfig.logConfig.filePath = p;
        ui_log_setLogPath(p);
    }
}

const SerialConfig MainWindow::ui_serial_getConfig()
{
    SerialConfig config;
    config.portName = ui_serial_getPortName();
    config.portDataBit = ui_serial_getDataBit();
    config.portFlow = ui_serial_getFlow();
    config.portBaud = ui_serial_getBaud();
    config.portParity = ui_serial_getParity();
    config.portStopBit = ui_serial_getStopBit();
    return config;
}

void MainWindow::ui_serial_addPortName(const QSerialPortInfo info)
{
    this->ui->cbbPort->addItem(ui_serial_makePortName(info));
}

void MainWindow::ui_serial_addPortNames(const QList<QSerialPortInfo> list)
{
    for(int i = 0; i < list.size(); i++){
        this->ui->cbbPort->addItem(QString::fromUtf8("[%1]%2").arg(list.at(i).portName()).arg(list.at(i).description()));
    }
}

void MainWindow::ui_serial_clearPortNameExceptIndex(int index)
{
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

void MainWindow::ui_serial_clearPortNameExceptCurrent()
{
    while(this->ui->cbbPort->count() > 1){
        this->ui->cbbPort->removeItem(1);
    }
}

void MainWindow::ui_serial_setPortName(QSerialPortInfo info)
{
    const QString pn = QString::fromUtf8("[%1]%2").arg(info.portName()).arg(info.description());
    this->ui->cbbPort->setCurrentText(pn);
}

void MainWindow::ui_serial_setBaud(qint64 baud)
{
    this->ui->cbbBaud->setCurrentText(QString::fromUtf8("%1").arg(baud));
}

void MainWindow::ui_serial_setDataBit(QSerialPort::DataBits d)
{
    this->ui->cbbDataBit->setCurrentIndex(d - this->ui->cbbDataBit->itemText(0).toInt());
}

void MainWindow::ui_serial_setParity(QSerialPort::Parity p)
{
    int pi = (int)p;
    if(pi > 0) pi -= 1;
    this->ui->cbbParity->setCurrentIndex(pi);
}

void MainWindow::ui_serial_setStopBit(QSerialPort::StopBits s)
{
    this->ui->cbbStop->setCurrentIndex(s - 1);
}

void MainWindow::ui_serial_setFlow(QSerialPort::FlowControl f)
{
    this->ui->cbbFlow->setCurrentIndex(f);
}

void MainWindow::ui_serial_setConfig(SerialConfig config)
{
    QSerialPortInfo info(config.portName);
    ui_serial_setPortName(info);
    ui_serial_setBaud(config.portBaud);
    ui_serial_setDataBit(config.portDataBit);
    ui_serial_setParity(config.portParity);
    ui_serial_setStopBit(config.portStopBit);
    ui_serial_setFlow(config.portFlow);
}

void MainWindow::ui_serial_toggle_pbtSend(bool _isOpen)
{
    if(_isOpen){
        this->ui->pbtSend->setText(QString::fromUtf8("发送"));
    }else{
        this->ui->pbtSend->setText(QString::fromUtf8("打开"));
    }
}

void MainWindow::ui_net_role_function(int protocol)
{
    QStringList strs = {QString::fromUtf8("Client"), QString::fromUtf8("Server"), QString::fromUtf8("多播"), QString::fromUtf8("广播")};
    if(NetWorkSettingConfig::TCP == protocol){
        this->ui->cbbNetRole->clear();
        for(int i = 0; i < 2; i++){
            this->ui->cbbNetRole->addItem(strs.at(i));
        }
        this->ui->cbbNetRole->setCurrentIndex(0);
    }
    if(NetWorkSettingConfig::UDP == protocol){
        this->ui->cbbNetRole->clear();
        this->ui->cbbNetRole->addItems(strs);
        this->ui->cbbNetRole->setCurrentIndex(0);
    }
}

void MainWindow::ui_net_addInterface(QNetworkInterface interface)
{
    this->ui->cbbNetworkInterface->addItem(ui_net_makeInterfaceStr(interface));
}

const QString MainWindow::ui_net_getCurrentInterfaceHardAddr()
{
    int ns = this->ui->cbbNetworkInterface->count();
    if(ns == 0) return QString("");
    const QString str = this->ui->cbbNetworkInterface->currentText();
    int pos = str.indexOf('[');
    int pos2 = str.indexOf(']');
    return str.mid(pos + 1, pos2 - pos - 1);
}

const QString MainWindow::ui_net_getCurrentInterfaceHumanNamme()
{
    int ns = this->ui->cbbNetworkInterface->count();
    if(ns == 0) return QString("");
    const QString str = this->ui->cbbNetworkInterface->currentText();
    int pos = str.indexOf('[');

    return str.mid(0, pos - 1);
}

const QString MainWindow::ui_net_getCurrentInterfaceAddr(bool ipv6)
{
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

void MainWindow::ui_net_setCurrentInterface(QNetworkInterface interface)
{
    const QString str = ui_net_makeInterfaceStr(interface);
    int find = this->ui->cbbNetworkInterface->findText(str);

    if(find != -1){
        this->ui->cbbNetworkInterface->setCurrentIndex(find);
    }else{
        this->ui->cbbNetworkInterface->addItem(str);
        this->ui->cbbNetworkInterface->setCurrentIndex(this->ui->cbbNetworkInterface->count() - 1);
    }
}

QNetworkInterface MainWindow::ui_net_getInterface(bool *ok)
{
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

bool MainWindow::ui_net_isEnableIPV6()
{
    return this->ui->cbEnableIPV6->isChecked();
}

int MainWindow::ui_net_getRole()
{
    return this->ui->cbbNetRole->currentIndex();
}

int MainWindow::ui_net_getProfile()
{
    return this->ui->cbbNetProfile->currentIndex();
}

int MainWindow::ui_net_getPort()
{
    return this->ui->sbSourcePort->value();
}

const QString MainWindow::ui_net_getIP()
{
    return this->ui->leIPSource->text();
}

void MainWindow::ui_net_setIP(const QString &ip)
{
    this->ui->leIPSource->setText(ip);
}

void MainWindow::ui_net_initStatusTable()
{
//    if(!clientModel) clientModel = new QStandardItemModel(0, 3);
//    clientModel->setHorizontalHeaderLabels(QStringList() << QString::fromUtf8("状态") << QString::fromUtf8("IP") << QString::fromUtf8("端口"));
//    this->ui->tbvClientConnection->setModel(clientModel);

    QHeaderView *header = nullptr;

    header = this->ui->tbwShowConnnectStatus->horizontalHeader();
    header->setDefaultAlignment(Qt::AlignCenter);
    header->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui_net_addConnectionToTable("127.255.255.255", "127.0.0.1", 0, 80);
    ui_net_addConnectionToTable("127.0.0.1", "127.255.255.255", 1, 8080);
    ui_net_addConnectionToTable("127.255.255.255", "127.0.0.1", 0, 8081);
    ui_net_addConnectionToTable("127.0.0.1", "127.255.255.255", 1, 9803);
    ui_net_addConnectionToTable("127.255.255.255", "127.0.0.1", 0, 3459);
    ui_net_addConnectionToTable("127.0.0.1", "127.255.255.255", 1, 65535);
}

void MainWindow::ui_net_addConnectionToTable(const QString &source_ip, const QString& des_ip,int status, int port)
{
//    if(!clientModel) return;
//    QList<QTableWidgetItem *> items;
    QList<QString> ins = {source_ip, QString::fromUtf8("%1").arg(status), des_ip, QString::fromUtf8("%1").arg(port)};
    if(status > 0){
        ins.replace(1, QString::fromUtf8("<--->"));
    }
    if(status == 0){
        ins.replace(1, QString::fromUtf8("<-/->"));
    }

//    for(int i = 0; i < ins.count(); i++){
//        items.append(new QTableWidgetItem(ins.at(i)));
//    }
    int row_count = this->ui->tbwShowConnnectStatus->rowCount();

//    this->ui->tbwShowConnnectStatus->setRowCount(count);

    this->ui->tbwShowConnnectStatus->insertRow(row_count);

    int col_count = ins.count();
    for(int i = 0; i < col_count; i++){
        QTableWidgetItem *item = new QTableWidgetItem(ins.at(i));
        item->setTextAlignment(Qt::AlignCenter);
        this->ui->tbwShowConnnectStatus->setItem(row_count, i, item);
    }
//    clientModel->appendRow(items);

}

void MainWindow::ui_showTime()
{
    const QString dStr = QDateTime::currentDateTime().toString("[yyyy-MM-dd:hh:mm:ss.zzz] ");
    QString hStr = QString::fromUtf8("<i style=\"color:#999999;\">%1</i>").arg(dStr);
    this->ui->pteShowRecieve->appendHtml(hStr);
    this->ui->pteShowRecieve->moveCursor(QTextCursor::End);
}

void MainWindow::ui_showSend(const QString &str, bool t)
{
    QString hStr;
    QString dStr;
    if(t){
        dStr = QDateTime::currentDateTime().toString("[yyyy-MM-dd:hh:mm:ss.zzz] ");
        hStr.append(QString::fromUtf8("<i style=\"color:#999999;\">%1</i>").arg(dStr));
    }

    if(settingConfig.logConfig.enableSaveLog){
        ui_log_logSaveToFile(QString(dStr + str));
    }

    if(settingConfig.showConfig.enableShowColor){
        hStr.append(QString::fromUtf8("<big style=\"color:%1;\">%2</big>").arg(settingConfig.showConfig.sendColor.name(QColor::HexRgb)).arg(str));
    }else{
        hStr.append(QString::fromUtf8("<big style=\"color:black;\">%1</big>").arg(str));
    }

    hStr.replace(QLatin1String("\n"), QLatin1String("<br />"));
    this->ui->pteShowRecieve->appendHtml(hStr);
    this->ui->pteShowRecieve->moveCursor(QTextCursor::End);
}

void MainWindow::ui_showRecieve(const QString &s, bool t)
{
    QString hStr;
    QString dStr;
    QString str;
    if(settingConfig.recConfig.bufferMode){
        if(settingConfig.recConfig.bufferSize  > (recieveBuffer.size() + str.size())){
            recieveBuffer.append(s.toUtf8());
            return;
        }else{
            str = QString::fromUtf8(recieveBuffer) + s;
            recieveBuffer.clear();
        }
    }else{
        if(!recieveBuffer.isEmpty()) recieveBuffer.clear();
        str = QString(s);
    }

    int pos = this->ui->pteShowRecieve->textCursor().position();
    if(pos > (settingConfig.showConfig.bufferSize << 10 << 10)){
        ui_clearRecieve();
    }

    if(t){
        dStr = QDateTime::currentDateTime().toString("[yyyy-MM-dd:hh:mm:ss.zzz] ");
        hStr.append(QString::fromUtf8("<i style=\"color:#999999;\">%1</i>").arg(dStr));
    }

    if(settingConfig.logConfig.enableSaveLog){
        ui_log_logSaveToFile(QString(dStr + str));
    }

    if(settingConfig.showConfig.enableShowColor){
        hStr.append(QString::fromUtf8("<big style=\"color:%1;\">%2</big>").arg(settingConfig.showConfig.recColor.name(QColor::HexRgb)).arg(str));
    }else{
        hStr.append(QString::fromUtf8("<big style=\"color:black;\">%1</big>").arg(str));
    }

    hStr.replace(QLatin1String(settingConfig.lineEnd[settingConfig.lineMode]), QLatin1String("<br />"));

    this->ui->pteShowRecieve->appendHtml(hStr);
    this->ui->pteShowRecieve->moveCursor(QTextCursor::End);
}

void MainWindow::ui_clearRecieve()
{
    this->ui->pteShowRecieve->clear();
    this->ui->pteShowRecieve->moveCursor(QTextCursor::Start);

    if(!recieveBuffer.isEmpty()) recieveBuffer.clear();

    txBytes = 0;
    rxBytes = 0;

    ui_statusbar_showTxBytes(txBytes);
    ui_statusbar_showRxBytes(rxBytes);
}

void MainWindow::ui_showRecieveData(const QByteArray &data, int _len)
{
    int len = _len;

    if(settingConfig.recConfig.showMode == ASCII_MODE){
        ui_showRecieve(QString::fromUtf8(data, len), settingConfig.showConfig.enableShowTime);
        return;
    }
    if(settingConfig.recConfig.showMode == HEX_MODE){
        const QByteArray hex = data.toHex().toUpper();
        len = hex.size();
        QByteArray bytes(len + len / 2 + 2, '\0');
        int i = 0, j = 0;
        for(i = 0; i < len;){
            bytes[j] = hex[i];
            bytes[j + 1] = hex[i + 1];
            bytes[j + 2] = ' ';
            j += 3;
            i += 2;
        }
        ui_showRecieve(QString::fromUtf8(bytes, j), settingConfig.showConfig.enableShowTime);
    }

}

//void MainWindow::ui_showSendData(const QByteArray &data, int len)
//{
//    if(settingConfig.sendConfig.sendMode == ASCII_MODE){
//        ui_showSend(QString::fromUtf8(data, len), settingConfig.showConfig.enableShowTime);
//        return;
//    }
//    if(settingConfig.sendConfig.sendMode == HEX_MODE){
//        const QByteArray hex = data.toHex().toUpper();
//        len = hex.size();
//        QByteArray bytes(len + len / 2 + 2, '\0');
//        int i = 0, j = 0;
//        for(i = 0; i < len;){
//            bytes[j] = hex[i];
//            bytes[j + 1] = hex[i + 1];
//            bytes[j + 2] = ' ';
//            j += 3;
//            i += 2;
//        }
//        ui_showSend(QString::fromUtf8(bytes, j), settingConfig.showConfig.enableShowTime);
//    }
//}

void MainWindow::ui_addSendHistory(const QString &str)
{
    if(this->ui->cbbSendHistory->findText(str) < 0)
        this->ui->cbbSendHistory->addItem(str);
}

void MainWindow::ui_addSendHistory(const QStringList &list)
{
//    this->ui->cbbSendHistory->addItems(list);
    for(int i = 0; i < list.size(); i++){
        ui_addSendHistory(list.at(i));
    }
}

void MainWindow::ui_clearSendHistory()
{
    while(this->ui->cbbSendHistory->count() > 0){
        this->ui->cbbSendHistory->removeItem(0);
    }
}

void MainWindow::ui_recieve_initRecieveFontColor()
{
//        QList<QColor> colors = {Qt::black, Qt::blue, Qt::darkGray,  Qt::white};
    QImage im(QSize(64, 32), QImage::Format_RGB32);
//        int s = colors.size();
    for(int i = 2; i < 19; i++){
        im.fill(Qt::GlobalColor(i));
        this->ui->cbbRecFontColor->addItem(QIcon(QPixmap::fromImage(im)), "");
    }
    this->ui->cbbRecFontColor->setCurrentIndex(7);
}

QColor MainWindow::ui_recieve_getRecieveFontColor()
{
    return Qt::GlobalColor(this->ui->cbbRecFontColor->currentIndex() + 2);
}

void MainWindow::ui_recieve_setRecieveFontColorState(bool state)
{
    this->ui->cbRecShowFontColor->setChecked(state);
}

void MainWindow::ui_setShowPlaintFont(const QFont &font)
{
    this->ui->pteSend->setFont(font);
    this->ui->pteShowRecieve->setFont(font);
}

int MainWindow::ui_serial_getPortNumber()
{
    return this->ui->cbbPort->count();
}

const QString MainWindow::ui_serial_getPortName()
{
    if(ui_serial_getPortNumber() == 0) return QString::fromUtf8("");
    const QString pn = this->ui->cbbPort->currentText();
    int pos = pn.indexOf(']');
    return pn.mid(1, pos - 1);
}

qint64 MainWindow::ui_serial_getBaud()
{
    return this->ui->cbbBaud->currentText().toLongLong();
}

QSerialPort::DataBits MainWindow::ui_serial_getDataBit()
{
    return QSerialPort::DataBits(this->ui->cbbDataBit->currentText().toInt());
}

QSerialPort::Parity MainWindow::ui_serial_getParity()
{
    int pi = this->ui->cbbParity->currentIndex();
    if(pi > 0){
        pi += 1;
    }
    return QSerialPort::Parity(pi);
}

QSerialPort::StopBits MainWindow::ui_serial_getStopBit()
{
    return QSerialPort::StopBits(this->ui->cbbStop->currentIndex() + 1);
}

QSerialPort::FlowControl MainWindow::ui_serial_getFlow()
{
    return QSerialPort::FlowControl(this->ui->cbbFlow->currentIndex());
}

int MainWindow::ui_recieve_getRecieveMode()
{
    if(this->ui->rbtRASCII->isChecked()){
        return ASCII_MODE;
    }
    if(this->ui->rbtRHex->isChecked()){
        return HEX_MODE;
    }
    return ASCII_MODE;
}

bool MainWindow::ui_show_isEnableAutoNewLine()
{
    return this->ui->cbAutoNewLine->isChecked();
}

bool MainWindow::ui_show_isEnableShowSend()
{
    return this->ui->cbShowSend->isChecked();
}

bool MainWindow::ui_show_isEnableShowTime()
{
    return this->ui->cbShowTime->isChecked();
}

bool MainWindow::ui_isEnableBufferMode()
{
    return this->ui->cbRecBufferMode->isChecked();
}

bool MainWindow::ui_show_isEnableShowColor()
{
    return this->ui->cbRecShowFontColor->isChecked();
}

int MainWindow::ui_recvieve_getBufferSize()
{
    return this->ui->sbRecBufferSize->value();
}

int MainWindow::ui_send_getSendMode()
{
    if(this->ui->rbtSASCII->isChecked()){
        return ASCII_MODE;
    }
    if(this->ui->rbtSHex->isChecked()){
        return HEX_MODE;
    }
    return ASCII_MODE;
}

bool MainWindow::ui_send_isEnableAutoRepeat()
{
    return this->ui->cbAutoResend->isChecked();
}

void MainWindow::ui_send_setAutoRepeatState(bool set)
{
    this->ui->cbAutoResend->setCheckState(set ? Qt::Checked : Qt::Unchecked);
}

int MainWindow::ui_send_getRepeatTime()
{
    return this->ui->sbRetime->value();
}

int MainWindow::ui_send_getRepeatTimeUnit()
{
    return this->ui->cbbRetimeUnit->currentIndex();
}

const QString MainWindow::ui_log_getLogPath()
{
    return this->ui->leLogPath->text();
}

bool MainWindow::ui_log_isEnableLog()
{
    return this->ui->cbEnableLog->isChecked();
}

bool MainWindow::event(QEvent *event)
{
    return QMainWindow::event(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{

    QMainWindow::resizeEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QMainWindow::mousePressEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if(this->ui->pteSend->underMouse()) qDebug() << e->key();
    QMainWindow::keyPressEvent(e);
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
//    qDebug() << e;
    QMainWindow::keyReleaseEvent(e);
}

void MainWindow::focusInEvent(QFocusEvent *event)
{
//    qDebug() << event;
    QMainWindow::focusInEvent(event);
}

void MainWindow::enterEvent(QEvent *event)
{
//    qDebug() << event;
    QMainWindow::enterEvent(event);
}

void MainWindow::actionEvent(QActionEvent *event)
{
//    qDebug() << event;
    QMainWindow::actionEvent(event);
}

