#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMainWindow>
#include <QApplication>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QFile>
#include <QTimer>
#include <QByteArray>
#include <QDebug>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>
#include "tcpudptranslator.h"
#include "datatransmitter.h"

#define ON                                      0x01
#define OFF                                     0x00

static const int decimalMass[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString getHostIP();

private:
    Ui::MainWindow *ui;

    TcpUdpTranslator        *td;
    TcpUdpTranslator        *td_1;
    DataTransmitter         *ud;
    QByteArray              ba;
    QTcpSocket              *tcpSocket;
    QTcpServer              *tcpServer;
    QUdpSocket              *udpSocket;
    QString                 hostIPAddr;
    uint                    hostPort;
    QString                 clientIPAddr;
    uint                    clientPort;
    QNetworkRequest         request;
    QNetworkAccessManager   *manager;
    QNetworkReply           *reply;

    char                    CRC = 0;

private slots:
    void on_tcpSendButton_clicked();
    void on_setSocketButton_clicked();
    void slotReadyRead();
    void slotConnected();
    void showTcpMessage();
    void showUdpMessage();
    void on_tryTelemetry_clicked();
};
#endif // MAINWINDOW_H
