#include <algorithm>
#include <vector>

#include "protocol.h"

#define UNUSED(x) (void)x;

namespace GroupFlight
{

#ifndef INTERFACE_H
#define INTERFACE_H

    class
            Handler
    {
    public:
        virtual ~Handler(){}
        virtual ErrorType setData(const std::vector<char> &data){UNUSED(data); return ErrorType::NoError;}
        virtual ErrorType setPackage(const Package &package){UNUSED(package); return ErrorType::NoError;}
    };

    class Interface : public Handler
    {
    public:
        virtual ~Interface(){}

        virtual void addHandler(Handler *handler)
        {
            if (!handler) return;
            handlers.push_back(handler);
        }

        virtual void removeHandler(Handler *handler)
        {
            if (!handler) return;

            auto index = std::find(handlers.begin(), handlers.end(), handler);
            if (index != handlers.end()) handlers.erase(index);
        }

        virtual size_t handlersCount(){ return handlers.size(); }

        virtual void setDataToHandlers(const std::vector<char> &data)
        {
            for (Handler *handler: handlers)
                handler->setData(data);
        }

        virtual void setPackageToHandlers(const Package &package)
        {
            for (Handler *handler: handlers)
                handler->setPackage(package);
        }

    private:
        std::vector<Handler*> handlers;

    };

#endif // INTERFACE_H

} // namespace GroupFlight
