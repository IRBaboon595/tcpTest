#include <QUdpSocket>
#include <QDebug>

#include "datatransmitter.h"

struct DataTransmitter::DataTransmitterPrivate
{
    QUdpSocket *socket = nullptr;
    QString host;
    QString client;
    quint16 portSrc = 1234;
    quint16 portDst = 4321;

    //QVector<GroupFlight::Handler*> listeners;
};

DataTransmitter::DataTransmitter():
    d(new DataTransmitterPrivate)
{
    d->socket = new QUdpSocket;
    QObject::connect(d->socket, &QUdpSocket::readyRead, [=]{this->dataReceived();});
}

DataTransmitter::~DataTransmitter()
{
    stop();

    delete d->socket;
    delete d;
}

void DataTransmitter::setPortIn(uint16_t port)
{
    d->portSrc = port;
}

uint16_t DataTransmitter::portIn()
{
    return d->portSrc;
}

void DataTransmitter::setPortOut(uint16_t port)
{
    d->portDst = port;
}

uint16_t DataTransmitter::portOut()
{
    return d->portDst;
}

void DataTransmitter::setHost(const std::string &host)
{
    d->host = QString::fromStdString(host);
}

void DataTransmitter::setHost(uint32_t host)
{
    d->host = QString::fromStdString(ipFromUint32(host));
}

std::string DataTransmitter::host()
{
    return d->host.toStdString();
}

bool DataTransmitter::start(const std::string &host, uint16_t portIn, uint16_t portOut)
{
    setHost(host);
    setPortIn(portIn);
    setPortOut(portOut);
    return start();
}

bool DataTransmitter::start(uint32_t host, uint16_t portIn, uint16_t portOut)
{
    return start(ipFromUint32(host), portIn, portOut);
}

bool DataTransmitter::start()
{
    if (!d->socket) return false;

    bool result = d->socket->bind(QHostAddress::AnyIPv4, d->portSrc,
                                  QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    if (QHostAddress(d->host).isMulticast())
    {
        static const QVariant multiCastUDPconst(1);

        d->socket->setSocketOption(QAbstractSocket::MulticastTtlOption, multiCastUDPconst);      //проблема с записью числа int '1'; решил её использовав статик конст QVariant со значением интовой 1
        d->socket->joinMulticastGroup(QHostAddress(d->host));
    }

    return result;
}

void DataTransmitter::stop()
{
    if (d->socket->state() == QAbstractSocket::BoundState)
        d->socket->close();
}

bool DataTransmitter::isStarted()
{
    return (d->socket->state() == QAbstractSocket::BoundState);
}

void DataTransmitter::sendData(const std::vector<char> &data)
{
    if (!d->socket) return;

    QByteArray msg(QByteArray::fromRawData(data.data(), data.size()));
    d->socket->writeDatagram(msg, QHostAddress(d->host), d->portDst);
}

/*void DataTransmitter::addListener(GroupFlight::Handler *listener)
{
    if (!listener) return;
    d->listeners.push_back(listener);
}

void DataTransmitter::removeListener(GroupFlight::Handler *listener)
{
    if (!d->listeners.contains(listener)) return;
    d->listeners.removeOne(listener);
}*/

uint32_t DataTransmitter::ipFromString(const char *strAddr)
{
    uint32_t result = 0;
    uint32_t byte0, byte1, byte2, byte3;
    char dummyString[2];

    if (sscanf(strAddr, "%u.%u.%u.%u%1s",
                &byte3, &byte2, &byte1, &byte0, dummyString) == 4)
    {
        if ((byte3 < 256) && (byte2 < 256) && (byte1 < 256) && (byte0 < 256))
            result = (byte3 << 24) + (byte2 << 16) + (byte1 << 8) + byte0;
    }

    return result;
}

std::string DataTransmitter::ipFromUint32(uint32_t addr)
{
    std::string result;
    result.reserve(15);
    result += std::to_string((addr >> 24) & 0xff);
    result += ".";
    result += std::to_string((addr >> 16) & 0xff);
    result += ".";
    result += std::to_string((addr >> 8) & 0xff);
    result += ".";
    result += std::to_string(addr & 0xff);

    return result;
}

void DataTransmitter::dataReceived()
{
    if (!d->socket) return;

    while (d->socket->hasPendingDatagrams())
    {
        QByteArray data;
        data.resize(d->socket->pendingDatagramSize());
        d->socket->readDatagram(data.data(), data.size());
        qDebug() << d->socket->localAddress();
        std::vector<char> msg(data.begin(), data.end());
        /*for (GroupFlight::Handler *listener: qAsConst(d->listeners))
            listener->setData(msg);*/
    }
}
