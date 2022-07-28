#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QString tempIP = getHostIP();
    td = new TcpUdpTranslator;

    td->setIPAddress(ProtocolType::TCP, DirectionType::Blank, "127.0.0.1");
    td->setPort(ProtocolType::TCP, DirectionType::Blank, 10003);
    td->connectToServer(ProtocolType::TCP);

    td_1 = new TcpUdpTranslator;

    td_1->setIPAddress(ProtocolType::TCP, DirectionType::Blank, "192.168.77.82");
    td_1->setPort(ProtocolType::TCP, DirectionType::Blank, 7071);
    td_1->connectToServer(ProtocolType::TCP);

    td->setPort(ProtocolType::UDP, DirectionType::Host, 8082);
    td->connectToServer(ProtocolType::UDP);

    connect(td, SIGNAL(tcpReceived()), this, SLOT(showTcpMessage()));
    connect(td, SIGNAL(udpReceived()), this, SLOT(showUdpMessage()));
}

/**************************************** Service Functions ****************************************/

QString MainWindow::getHostIP()
{
    QUdpSocket tempSock;
    QString localIP="127.0.0.1";    //fall back
    QString googleDns = "8.8.8.83";  //try google DNS or sth. else reliable first
    tempSock.connectToHost(googleDns, 53);
    if (tempSock.waitForConnected(3000))
    {
        localIP = tempSock.localAddress().toString();
    }
    else
    {
        foreach (const QHostAddress &address, QNetworkInterface::allAddresses())
        {
            QString guessedGatewayAddress = address.toString().section( ".",0,2 ) + ".1";

            if (address.protocol() == QAbstractSocket::IPv4Protocol
                    && address != QHostAddress(QHostAddress::LocalHost)
                    )
            {
                tempSock.connectToHost(guessedGatewayAddress, 53);
                if (tempSock.waitForConnected(3000))
                {
                    localIP = tempSock.localAddress().toString();
                    break;
                }
            }
        }
    }
    return localIP;
}

/***************************************** Control Functions *********************************************/

void MainWindow::on_tcpSendButton_clicked()
{
    td->write(ProtocolType::TCP, td->sendTelemetryRequest());
    td->write(ProtocolType::UDP, ui->udpDataLineEdit->text().toUtf8());
}

void MainWindow::on_setSocketButton_clicked()
{
    QString str = ui->ip4LineEdit_3->text() + '.' + ui->ip4LineEdit_2->text() + '.' + ui->ip4LineEdit_1->text() + '.' + ui->ip4LineEdit_0->text();
    td->setPort(ProtocolType::UDP, DirectionType::Client, ui->udpPortLineEdit->text().toUInt());
    td->setIPAddress(ProtocolType::UDP, DirectionType::Client, str);
    //td->connectToServer(ProtocolType::UDP);
}

void MainWindow::slotReadyRead()
{
    qDebug() << reply->readAll();
}

void MainWindow::slotConnected()
{
    qDebug() << "Received the connected() signal";
    td->write(ProtocolType::TCP, td->sendTelemetryRequest());
}

void MainWindow::showTcpMessage()
{
    QByteArray tempBa;
    tempBa.clear();
    ui->tcpDataLabel->setText(td->ByteData());
    td->setByteData(tempBa);
}

void MainWindow::showUdpMessage()
{
    QByteArray tempBa;
    tempBa.clear();
    ui->udpDataLabel->setText(td->ByteData());
    td->setByteData(tempBa);
}

void MainWindow::on_tryTelemetry_clicked()
{
    td->write(ProtocolType::TCP, td->sendTelemetryRequest());
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*void tcpUdpTranslator::tcpPendingConnection()
{
    socket = server->nextPendingConnection();
    qDebug() << "Socket opened";
    //socket->connectToHost(hostIPAddr, hostPort, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);
    socket->open(QIODevice::ReadWrite);
    qDebug() << socket->state();
    QByteArray Data;
    Data.append(ui->udpDataLineEdit->text().toUtf8());
    qDebug() << server->serverPort();
    socket->write(Data);
    connect(socket, SIGNAL(readyRead()), this, SLOT(tcpRead()));
}*/




