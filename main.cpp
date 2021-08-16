#include "mainwindow.h"

#include <QApplication>
#include <QThread>

#include "main.h"

#ifdef SERIAL_THRAED
#include "serialthread.h"
#else

#endif

#ifdef NETWORK_THREAD
#include "tcpthread.h"
#include "udpthread.h"
#endif

SerialConfig serialConfig;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
#ifdef    SERIAL_THREAD
    QThread sThread;
    SerialThread *serialThread = new SerialThread();
#endif

#ifdef NETWORK_THREAD
    QThread tThread;
    QThread uThread;
    TCPThread *tcpThread = new TCPThread();
    UDPThread *udpThread = new UDPThread();
#endif
#ifdef SERIAL_THREAD
//    qDebug() << "Main:" << QThread::currentThread();
    QObject::connect(&w, &MainWindow::ui_serial_config_changed, serialThread, &SerialThread::handle_serial_changed);
    QObject::connect(&w, &MainWindow::ui_serial_open, serialThread, &SerialThread::handle_serial_open);
    QObject::connect(&w, &MainWindow::ui_serial_close, serialThread, &SerialThread::handle_serial_close);
    QObject::connect(&w, &MainWindow::ui_serial_start, serialThread, &SerialThread::handle_serial_start);
    QObject::connect(&w, &MainWindow::ui_serial_pause, serialThread, &SerialThread::handle_serial_pause);
    QObject::connect(&w, &MainWindow::ui_serial_send, serialThread, &SerialThread::handle_serial_send);

    QObject::connect(serialThread, &SerialThread::serialErrorOccurred, &w, &MainWindow::handle_serial_error);
    QObject::connect(serialThread, &SerialThread::serialdataRecieveComplete, &w, &MainWindow::handle_serial_recieve_data);

    serialThread->moveToThread(&sThread);
    sThread.start();
#endif

#ifdef NETWORK_THREAD
    // TCP
    QObject::connect(&w, &MainWindow::ui_tcp_start, tcpThread, &TCPThread::tcp_start);
    QObject::connect(&w, &MainWindow::ui_tcp_stop, tcpThread, &TCPThread::tcp_stop);
    QObject::connect(&w, &MainWindow::ui_tcp_send, tcpThread, &TCPThread::tcp_send);

    QObject::connect(tcpThread, &TCPThread::tcp_server_started, &w, &MainWindow::handle_tcp_server_started);
    QObject::connect(tcpThread, &TCPThread::tcp_server_new_connection, &w, &MainWindow::handle_tcp_server_new_connection);
    QObject::connect(tcpThread, &TCPThread::tcp_server_client_disconnected, &w, &MainWindow::handle_tcp_server_client_disconnected);
    QObject::connect(tcpThread, &TCPThread::tcp_server_occur_error, &w, &MainWindow::handle_tcp_server_occur_error);
    QObject::connect(tcpThread, &TCPThread::tcp_server_recieved_data, &w, &MainWindow::handle_tcp_server_recieved_data);

    QObject::connect(tcpThread, &TCPThread::tcp_client_connected, &w, &MainWindow::handle_tcp_client_connected);
    QObject::connect(tcpThread, &TCPThread::tcp_client_disconnected, &w, &MainWindow::handle_tcp_client_disconnected);
    QObject::connect(tcpThread, &TCPThread::tcp_client_occur_error, &w, &MainWindow::handle_tcp_client_occur_error);
    QObject::connect(tcpThread, &TCPThread::tcp_client_recieved_data, &w, &MainWindow::handle_tcp_client_recieved_data);

    tcpThread->moveToThread(&tThread);
    tThread.start();
#endif


    //UDP


    w.setWindowTitle(QString::fromUtf8("串口助手"));

    w.show();
    int res = a.exec();

#ifdef SERIAL_THREAD
    sThread.terminate();
    sThread.wait();
#endif
#ifdef NETWORK_THREAD
    tThread.exit();
    tThread.wait();

    uThread.exit();
    uThread.wait();
#endif

    return res;
}
