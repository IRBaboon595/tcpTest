#ifndef LIBGROUPFLIGHT_H
#define LIBGROUPFLIGHT_H

#include "interface.h"

extern "C"
{
    void initLibrary(GroupFlight::Handler *handler);
    void setData(GroupFlight::Handler *handler, const std::vector<char> &data);
    void setPackage(GroupFlight::Handler *handler, const GroupFlight::Package &package);
    void stopLibrary(GroupFlight::Handler *handler);
}

#endif // LIBGROUPFLIGHT_H
