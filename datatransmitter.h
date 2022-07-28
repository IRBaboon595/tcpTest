#ifndef DATATRANSMITTER_H
#define DATATRANSMITTER_H

#include <vector>
#include <string>
//#include "interface.h"
#include <QVariant>

class DataTransmitter
{
public:
    DataTransmitter();
    virtual ~DataTransmitter();

    void setPortIn(uint16_t port);
    uint16_t portIn();

    void setPortOut(uint16_t port);
    uint16_t portOut();

    void setHost(const std::string &host);
    void setHost(uint32_t host);
    std::string host();

    void setUseMulticast(bool state = false);

    bool start(const std::string &host, uint16_t portIn, uint16_t portOut);

    bool start(uint32_t host, uint16_t portIn, uint16_t portOut);

    bool start();
    void stop();

    bool isStarted();

    void sendData(const std::vector<char> &data);

    //void addListener(GroupFlight::Handler *listener);
    //void removeListener(GroupFlight::Handler *listener);

    static uint32_t ipFromString(const char* strAddr);
    static std::string ipFromUint32(uint32_t addr);

private:
    void dataReceived();

    struct DataTransmitterPrivate;
    DataTransmitterPrivate * const d;
};

#endif // DATATRANSMITTER_H
