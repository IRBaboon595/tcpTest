#include "tcpudptranslator.h"

TcpUdpTranslator::TcpUdpTranslator(QObject *parent)
    : QObject{parent}
{
    m_ba.clear();
    m_tcpServerIPAddr.clear();
    m_udpHostIPAddr.clear();
    m_tcpServerPort = 0;
    m_udpDstPort = 0;
    m_tcpSocket = new QTcpSocket;
    m_udpSocket = new QUdpSocket;
    m_apType = AutopilotProtocol::BoardTelemetry;
}

void TcpUdpTranslator::setIPAddress(ProtocolType type, DirectionType direction, QString address)
{
    switch (type) {
    case ProtocolType::TCP:
        this->m_tcpServerIPAddr = address;
        break;
    case ProtocolType::UDP:
        if(direction == DirectionType::Host)
            this->m_udpHostIPAddr = address;
        break;
    }
}

void TcpUdpTranslator::setPort(ProtocolType type, DirectionType direction, uint port)
{
    switch (type) {
    case ProtocolType::TCP:
        this->m_tcpServerPort = port;
        break;
    case ProtocolType::UDP:
        if(direction == DirectionType::Host)
        {
            this->m_udpDstPort = port;
        }
        else if(direction == DirectionType::Client)
        {
            this->m_udpSrcPort = port;
        }
        break;
    }
}

void TcpUdpTranslator::setByteData(QByteArray ba)
{
    this->m_ba = ba;
}

void TcpUdpTranslator::setTelemetry(GroupFlight::Telemetry telemetry)
{
    this->m_telemetry = telemetry;
}

void TcpUdpTranslator::setAPType(AutopilotProtocol type)
{
    this->m_apType = type;
}

void TcpUdpTranslator::setHomePoint(GroupFlight::Coords point)
{
    this->m_homePoint = point;
}

void TcpUdpTranslator::setCurrentPoint(GroupFlight::Coords point)
{
    this->m_currentPoint = point;
}

void TcpUdpTranslator::setRoute(std::vector<GroupFlight::FlightPoint> route)
{
    this->m_route = route;
}

QString TcpUdpTranslator::IPAddress(ProtocolType type, DirectionType direction)
{
    QString result = "blank";
    switch (type) {
    case ProtocolType::TCP:
        result = this->m_tcpServerIPAddr;
        break;
    case ProtocolType::UDP:
        if(direction == DirectionType::Host)
            result = this->m_udpHostIPAddr;
        break;
    }
    return result;
}

uint TcpUdpTranslator::Port(ProtocolType type, DirectionType direction)
{
    uint result = 0;
    switch (type) {
    case ProtocolType::TCP:
        result = this->m_tcpServerPort;
        break;
    case ProtocolType::UDP:
        if(direction == DirectionType::Host)
        {
            result = this->m_udpDstPort;
        }
        else if(direction == DirectionType::Client)
        {
            result = this->m_udpSrcPort;
        }
        break;
    }
    return result;
}

QByteArray TcpUdpTranslator::ByteData()
{
    return this->m_ba;
}

GroupFlight::Telemetry TcpUdpTranslator::telemetry()
{
    return m_telemetry;
}

AutopilotProtocol TcpUdpTranslator::apType()
{
    return m_apType;
}

GroupFlight::Coords TcpUdpTranslator::homePoint()
{
    return m_homePoint;
}

GroupFlight::Coords TcpUdpTranslator::currentPoint()
{
    return m_currentPoint;
}

std::vector<GroupFlight::FlightPoint> TcpUdpTranslator::route()
{
    return m_route;
}

void TcpUdpTranslator::connectToServer(ProtocolType type)
{
    switch (type) {
    case ProtocolType::TCP:
        QObject::connect(m_tcpSocket, &QTcpSocket::connected, [=]{this->slotConnected(m_apType);});
        m_tcpSocket->connectToHost(m_tcpServerIPAddr, m_tcpServerPort, QIODevice::ReadWrite, QAbstractSocket::IPv4Protocol);
        break;
    case ProtocolType::UDP:
        if(startUdp())
            QObject::connect(m_udpSocket, &QUdpSocket::readyRead, [=]{this->dataRead(ProtocolType::UDP);});
        break;
    }
}

void TcpUdpTranslator::write(ProtocolType type, QByteArray data)
{
    switch (type) {
    case ProtocolType::TCP:
        this->m_tcpSocket->write(data);
        break;
    case ProtocolType::UDP:
        this->m_udpSocket->writeDatagram(data, QHostAddress(m_udpHostIPAddr), m_udpDstPort);
        break;
    }
}

bool TcpUdpTranslator::startUdp()
{
    if (!m_udpSocket) return false;

    bool result = m_udpSocket->bind(QHostAddress::AnyIPv4, m_udpSrcPort,
                                  QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    if (QHostAddress("239.1.2.3").isMulticast())
    {
        static const QVariant multiCastUDPconst(1);

        m_udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, multiCastUDPconst);      //проблема с записью числа int '1'; решил её использовав статик конст QVariant со значением интовой 1
        m_udpSocket->joinMulticastGroup(QHostAddress("239.1.2.3"));
    }

    return result;
}

void TcpUdpTranslator::slotConnected(AutopilotProtocol prot)
{
    qDebug() << "Received the connected() signal";
    QObject::connect(m_tcpSocket, &QTcpSocket::readyRead, [=]{this->dataRead(ProtocolType::TCP);});
    switch (prot) {
    case AutopilotProtocol::BoardTelemetry:
        qDebug() << "Telemetry requested";
        write(ProtocolType::TCP, sendTelemetryRequest());
        break;
    case AutopilotProtocol::RoutePoints:
        qDebug() << "Route requested";
        write(ProtocolType::TCP, sendRoutePointsRequest());
        break;
    case AutopilotProtocol::Supervisor:
        qDebug() << "Supervisor Telemetry requested";
        write(ProtocolType::TCP, sendSupervisorRequest());
        break;
    }
}

void TcpUdpTranslator::dataRead(ProtocolType type)
{
    QVariantMap map;
    uint curPoint = 0;
    uint totalPoints = 0;
    QList<QVariant> tempList;
    QVariantMap tempMap;
    QByteArray tempBa;

    switch (type) {
    case ProtocolType::TCP:

        tempBa = m_tcpSocket->readAll();

        switch (m_apType) {
        case AutopilotProtocol::BoardTelemetry:

            map = MsgPack::unpack(m_tcpSocket->readAll()).toMap();

            if (map.isEmpty()) return;

            if (!map.contains(QLatin1String("latitude")) ||
                !map.contains(QLatin1String("longitude")) ||
                !map.contains(QLatin1String("altitude"))) return;

            m_telemetry.lat = map.value("latitude").toDouble();
            m_telemetry.lon = map.value("longitude").toDouble();
            m_telemetry.alt = map.value("altitude").toFloat();
            m_telemetry.pitch = GroupFlight::radToDeg(map.value("pitch").toFloat());
            m_telemetry.roll = GroupFlight::radToDeg(map.value("roll").toFloat());
            m_telemetry.course = GroupFlight::radToDeg(map.value("azimuth").toFloat());
            m_telemetry.speed = map.value("speed").toFloat();
            m_telemetry.dateTime = uint32_t(QDateTime::currentDateTime().toSecsSinceEpoch());
            break;
        case AutopilotProtocol::RoutePoints:

            map = MsgPack::unpack(tempBa).toMap();

            if (map.isEmpty()) return;

            if (!map.contains(QLatin1String("count")) ||
                !map.contains(QLatin1String("current")) ||
                !map.contains(QLatin1String("points"))) return;

            m_route.clear();
            curPoint = map.value("current").toUInt();
            totalPoints = map.value("count").toUInt();
            tempList = map.value("points").toList();

            for(uint i = 0; i < (totalPoints + 1); i++)
            {
                tempMap = tempList.at(i).toMap();
                m_route.push_back(GroupFlight::FlightPoint(GroupFlight::Coords(i, tempMap.value("lat").toDouble() * GroupFlight::kPi / 180,
                                                                               tempMap.value("lon").toDouble() * GroupFlight::kPi / 180,
                                                                               tempMap.value("alt").toDouble())));
                if(i == 0) m_homePoint = m_route.at(0).point;
                if(i == curPoint) m_currentPoint = m_route.at(i).point;
            }
            qDebug() << "Got Route";
            break;
        case AutopilotProtocol::Supervisor:
            m_ba = tempBa;
            break;
        }

        emit tcpReceived();
        break;
    case ProtocolType::UDP:
        if (!m_udpSocket) return;

        QHostAddress sender;
        quint16 senderPort;
        //QByteArray data;

        while (m_udpSocket->hasPendingDatagrams())
        {
            this->m_ba.resize(m_udpSocket->pendingDatagramSize());
            m_udpSocket->readDatagram(this->m_ba.data(), this->m_ba.size(), &sender, &senderPort);
        }

        qDebug() << "Message from: " << sender.toString();
        qDebug() << "Message port: " << senderPort;
        qDebug() << "Message: " << this->m_ba;

        emit udpReceived();
        break;
    }
}

QByteArray TcpUdpTranslator::sendTelemetryRequest()
{
    QJsonObject msg = {
        { "type",       "Board" }, //GroupFlightModule
        { "id",         4 },
        { "protocol",   "BoardTelemetry" }  //Supervisor BoardTelemetry
    };

    QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact);
    return data;
}

QByteArray TcpUdpTranslator::sendSupervisorRequest()
{
    QJsonObject msg = {
        { "type",       "Board" }, //GroupFlightModule
        { "id",         3 },
        { "protocol",   "Supervisor" }  //Supervisor BoardTelemetry
    };

    QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact);
    return data;
}

QByteArray TcpUdpTranslator::sendRoutePointsRequest()
{
    QJsonObject msg = {
        { "type",       "Board" }, //GroupFlightModule
        { "id",         4 },
        { "protocol",   "RoutePoints" }  //Supervisor BoardTelemetry
    };

    QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact);
    return data;
}

TcpUdpTranslator::~TcpUdpTranslator()
{
    this->m_tcpSocket->close();
    qDebug() << "Close";
    this->m_udpSocket->close();
}
