#ifndef TCPUDPTRANSLATOR_H
#define TCPUDPTRANSLATOR_H

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
#include <QObject>
#include <msgpack.h>
#include "GroupFlightGlobal/interface.h"
#include "GroupFlightGlobal/coords.h"

//! TCP или UDP
enum class ProtocolType : uint8_t
{
    TCP,
    UDP
};

enum class DirectionType : uint8_t
{
    Host,
    Client,
    Blank
};

enum class AutopilotProtocol : uint8_t
{
    Supervisor,
    BoardTelemetry,
    RoutePoints
};

typedef union{
    uint16_t 	istd;
    uint8_t 	cstd[2];
}std_union;

typedef union{
    uint32_t 	listd;
    uint16_t 	istd[2];
    uint8_t 	cstd[4];
}long_std_union;

typedef union{
    uint64_t 	llistd;
    uint32_t 	listd[2];
    uint16_t 	istd[4];
    uint8_t 	cstd[8];
}long_long_std_union;

class TcpUdpTranslator : public QObject
{
    Q_OBJECT
public:
    explicit TcpUdpTranslator(QObject *parent = nullptr);
    ~TcpUdpTranslator();

    void setIPAddress(ProtocolType type, DirectionType direction, QString address);
    void setPort(ProtocolType type, DirectionType direction, uint port);
    void setByteData(QByteArray ba);
    void setTelemetry(GroupFlight::Telemetry telemetry);
    void setAPType(AutopilotProtocol type);
    void setHomePoint(GroupFlight::Coords point);
    void setCurrentPoint(GroupFlight::Coords point);
    void setRoute(std::vector<GroupFlight::FlightPoint> route);
    QString IPAddress(ProtocolType type, DirectionType direction);
    uint Port(ProtocolType type, DirectionType direction);
    QByteArray ByteData();
    GroupFlight::Telemetry telemetry();
    AutopilotProtocol apType();
    GroupFlight::Coords homePoint();
    GroupFlight::Coords currentPoint();
    std::vector<GroupFlight::FlightPoint> route();

    void connectToServer(ProtocolType type);
    void write(ProtocolType type, QByteArray data);
    bool startUdp();
    QByteArray sendTelemetryRequest();
    QByteArray sendSupervisorRequest();
    QByteArray sendRoutePointsRequest();

private:
    QByteArray                              m_ba;
    QTcpSocket                              *m_tcpSocket;
    QUdpSocket                              *m_udpSocket;
    QString                                 m_tcpServerIPAddr;
    uint                                    m_tcpServerPort;
    QString                                 m_udpHostIPAddr;
    uint                                    m_udpDstPort;
    uint                                    m_udpSrcPort;
    GroupFlight::Telemetry                  m_telemetry;
    std::vector<GroupFlight::FlightPoint>   m_route;
    GroupFlight::Coords                     m_homePoint;            // Точка дом
    GroupFlight::Coords                     m_currentPoint;         // Текущая точка маршрута
    AutopilotProtocol                       m_apType;

public slots:
    void slotConnected(AutopilotProtocol prot);
    void dataRead(ProtocolType type);

signals:
    void tcpReceived();
    void udpReceived();
};

#endif // TCPUDPTRANSLATOR_H
