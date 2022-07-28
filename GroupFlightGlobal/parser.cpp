#include <cstring>

#include "parser.h"
#include "protocol.h"

namespace GroupFlight
{

#ifndef GF_PARSER_CPP
#define GF_PARSER_CPP

    static const uint8_t k_minPackageSize = 19;     // Минимальный размер пакета
    static const uint64_t k_maxPackageSize = 65535; // Максимальный размер пакета
    static const uint8_t k_headerSize = 15;         // Размер заголовка пакета
    static const uint8_t k_valueSize = 3;           // Размер пары "Ключ-Значение"
    static const uint8_t k_crcSize = 4;             // Размер контрольной суммы
    static const char k_headSymbol1 = 0xB0;         // Символ заголовка 1
    static const char k_headSymbol2 = 0x3B;         // Символ заголовка 2
    static const char k_headSymbol3 = 0x7E;         // Символ заголовка 3

    unsigned int crc32(const char *buf, unsigned long len)
    {
        unsigned long crc_table[256];
        unsigned long crc;
        for (int i = 0; i < 256; i++)
        {
            crc = i;
            for (int j = 0; j < 8; j++)
                crc = (crc & 1) ? ((crc >> 1) ^ 0xEDB88320UL) : (crc >> 1);
            crc_table[i] = crc;
        };
        crc = 0xFFFFFFFFUL;
        while (len--)
            crc = crc_table[(crc ^ static_cast<unsigned char>(*buf++)) & 0xFF] ^ (crc >> 8);
        return crc ^ 0xFFFFFFFFUL;
    }

    void toPairs(const std::vector<FlightPoint> &fPoints, std::vector<Pair> &result)
    {
        result.clear();
        result.reserve(9 * fPoints.size());
        result.push_back(Pair(DataKey::PointsCount, fPoints.size()));

        for (const FlightPoint &fp: fPoints)
        {
            result.push_back(Pair(DataKey::PointNumber, fp.point.num));

            const int32_t &lat = fp.point.lat * 10000000;
            result.push_back(Pair(DataKey::LatitudeLowByte, (lat >> 16) & 0xffff));
            result.push_back(Pair(DataKey::LatitudeHighByte, lat & 0xffff));

            const int32_t &lon = fp.point.lon * 10000000;
            result.push_back(Pair(DataKey::LongitudeLowByte, (lon >> 16) & 0xffff));
            result.push_back(Pair(DataKey::LongitudeHighByte, lon & 0xffff));

            result.push_back(Pair(DataKey::Altitude, static_cast<uint16_t>(fp.point.alt)));
            result.push_back(Pair(DataKey::HoldRadius, fp.holdRadius));
            result.push_back(Pair(DataKey::HoldTime, fp.holdTime));

            result.push_back(Pair(static_cast<DataKey>(0xff), 0xffff));
        }
    }

    void toPairs(const std::vector<Coords> &points, std::vector<Pair> &result)
    {
        result.clear();
        result.reserve(7 * points.size());

        for (const Coords &point: points)
        {
            result.push_back(Pair(DataKey::PointNumber, point.num));

            const int32_t &lat = point.lat * 10000000;
            result.push_back(Pair(DataKey::LatitudeLowByte, (lat >> 16) & 0xffff));
            result.push_back(Pair(DataKey::LatitudeHighByte, lat & 0xffff));

            const int32_t &lon = point.lon * 10000000;
            result.push_back(Pair(DataKey::LongitudeLowByte, (lon >> 16) & 0xffff));
            result.push_back(Pair(DataKey::LongitudeHighByte, lon & 0xffff));

            result.push_back(Pair(DataKey::Altitude, static_cast<uint16_t>(point.alt)));
            result.push_back(Pair(static_cast<DataKey>(0xff), 0xffff));
        }
    }

    void toPairs(const AreaAfs &area, std::vector<Pair> &result)
    {
        result.clear();
        result.reserve(6 * area.points.size() + 4);

        for (const Coords &point: area.points)
        {
            result.push_back(Pair(DataKey::PointNumber, point.num));

            const int32_t &lat = point.lat * 10000000;
            result.push_back(Pair(DataKey::LatitudeLowByte, (lat >> 16) & 0xffff));
            result.push_back(Pair(DataKey::LatitudeHighByte, lat & 0xffff));

            const int32_t &lon = point.lon * 10000000;
            result.push_back(Pair(DataKey::LongitudeLowByte, (lon >> 16) & 0xffff));
            result.push_back(Pair(DataKey::LongitudeHighByte, lon & 0xffff));

            result.push_back(Pair(static_cast<DataKey>(0xff), 0xffff));
        }

        result.push_back(Pair(DataKey::Altitude, static_cast<uint16_t>(area.altitude)));
        result.push_back(Pair(DataKey::CrossOverlap, area.crossOverlap));
        result.push_back(Pair(DataKey::AlongOverlap, area.alongOverlap));

        if (area.resolution > 0)
            result.push_back(Pair(DataKey::Resolution, area.resolution));
    }

    void toPairs(const AreaRln &area, std::vector<Pair> &result)
    {
        result.clear();
        result.reserve(6 * area.points.size() + 4);

        for (const Coords &point: area.points)
        {
            result.push_back(Pair(DataKey::PointNumber, point.num));

            const int32_t &lat = point.lat * 10000000;
            result.push_back(Pair(DataKey::LatitudeLowByte, (lat >> 16) & 0xffff));
            result.push_back(Pair(DataKey::LatitudeHighByte, lat & 0xffff));

            const int32_t &lon = point.lon * 10000000;
            result.push_back(Pair(DataKey::LongitudeLowByte, (lon >> 16) & 0xffff));
            result.push_back(Pair(DataKey::LongitudeHighByte, lon & 0xffff));

            result.push_back(Pair(static_cast<DataKey>(0xff), 0xffff));
        }

        result.push_back(Pair(DataKey::Altitude, static_cast<uint16_t>(area.altitude)));
        result.push_back(Pair(DataKey::Overlap, static_cast<uint16_t>(area.overlap)));
        result.push_back(Pair(DataKey::FlightDistance, area.distance));
        result.push_back(Pair(DataKey::DataFormat, static_cast<uint16_t>(area.format)));
    }

    void toPairs(const ShootPoint &point, std::vector<Pair> &result)
    {
        result.clear();
        result.reserve(7);

        const int32_t &lat = point.point.lat * 10000000;
        result.push_back(Pair(DataKey::LatitudeLowByte, (lat >> 16) & 0xffff));
        result.push_back(Pair(DataKey::LatitudeHighByte, lat & 0xffff));

        const int32_t &lon = point.point.lon * 10000000;
        result.push_back(Pair(DataKey::LongitudeLowByte, (lon >> 16) & 0xffff));
        result.push_back(Pair(DataKey::LongitudeHighByte, lon & 0xffff));

        result.push_back(Pair(DataKey::Altitude, static_cast<uint16_t>(point.point.alt)));
        result.push_back(Pair(DataKey::FocalLength, point.focalLength));
        result.push_back(Pair(DataKey::FlyAroundFlag, point.flyAround));
    }

    void toPairs(const TrackerEnable &value, std::vector<Pair> &result)
    {
        result.clear();
        result.reserve(3);

        result.push_back(Pair(DataKey::ImageCoordinateX, value.x));
        result.push_back(Pair(DataKey::ImageCoordinateY, value.y));
        result.push_back(Pair(DataKey::TrackerEnableFlag, static_cast<uint16_t>(value.enable)));
    }

    void toPairs(const GroupMode &groupMode, std::vector<Pair> &result)
    {
        result.clear();
        result.reserve(5);

        result.push_back(Pair(DataKey::GroupFlightMode, static_cast<uint16_t>(groupMode.mode)));
        result.push_back(Pair(DataKey::MasterFlag, static_cast<uint16_t>(groupMode.master)));
        result.push_back(Pair(DataKey::DistancingX, static_cast<uint16_t>(groupMode.distancingX)));
        result.push_back(Pair(DataKey::DistancingY, static_cast<uint16_t>(groupMode.distancingY)));
        result.push_back(Pair(DataKey::DistancingZ, static_cast<uint16_t>(groupMode.distancingZ)));
    }

    void toPairs(const SelfId &selfId, std::vector<Pair> &result)
    {
        result.clear();
        result.reserve(2);

        result.push_back(Pair(DataKey::NumberUAV, selfId.number));
        result.push_back(Pair(DataKey::TypeCO, static_cast<uint16_t>(selfId.type)));
    }

    void toPairs(const Telemetry &telemetry, std::vector<Pair> &result)
    {
        result.clear();
        result.reserve(14);

        const int32_t &lat = telemetry.lat * 10000000;
        result.push_back(Pair(DataKey::LatitudeLowByte, (lat >> 16) & 0xffff));
        result.push_back(Pair(DataKey::LatitudeHighByte, lat & 0xffff));

        const int32_t &lon = telemetry.lon * 10000000;
        result.push_back(Pair(DataKey::LongitudeLowByte, (lon >> 16) & 0xffff));
        result.push_back(Pair(DataKey::LongitudeHighByte, lon & 0xffff));

        result.push_back(Pair(DataKey::AltitudeGPS, static_cast<uint16_t>(telemetry.alt)));

        const int16_t &pitch = static_cast<uint16_t>(telemetry.pitch * 10);
        result.push_back(Pair(DataKey::Pitch, pitch));

        const int16_t &roll = static_cast<uint16_t>(telemetry.roll * 10);
        result.push_back(Pair(DataKey::Roll, roll));

        const uint16_t &course = static_cast<uint16_t>(telemetry.course * 10);
        result.push_back(Pair(DataKey::Course, course));

        result.push_back(Pair(DataKey::Speed, static_cast<uint16_t>(telemetry.speed)));

        result.push_back(Pair(DataKey::FlightTimeLeft, telemetry.flightTimeLeft));
        result.push_back(Pair(DataKey::GroupFlightStatus, static_cast<uint16_t>(telemetry.groupFlightStatus)));

        const uint32_t &dateTime = telemetry.dateTime;
        result.push_back(Pair(DataKey::TimeDateLowByte, (dateTime >> 16) & 0xffff));
        result.push_back(Pair(DataKey::TimeDateHighByte, dateTime & 0xffff));

        result.push_back(Pair(DataKey::BoartStatus, static_cast<uint16_t>(telemetry.boardStatus)));
        result.push_back(Pair(DataKey::CurrentPoint, telemetry.currentPoint));
    }

    void toPairs(const NetworkParams &params, std::vector<Pair> &result)
    {
        result.clear();
        result.reserve(4);

        result.push_back(Pair(DataKey::PortIn, params.portIn));
        result.push_back(Pair(DataKey::PortOut, params.portOut));
        result.push_back(Pair(DataKey::HostLowByte, (params.host >> 16) & 0xffff));
        result.push_back(Pair(DataKey::HostHighByte, params.host & 0xffff));
    }

    void toPairs(const ManualControl &control, std::vector<Pair> &result)
    {
        result.clear();
        result.reserve(6);

        result.push_back(Pair(DataKey::MoveLeftFlag,    static_cast<uint16_t>(control.moveLeft)));
        result.push_back(Pair(DataKey::MoveRightFlag,   static_cast<uint16_t>(control.moveRight)));
        result.push_back(Pair(DataKey::MoveUpFlag,      static_cast<uint16_t>(control.moveUp)));
        result.push_back(Pair(DataKey::HoldCourseFlag,  static_cast<uint16_t>(control.holdCourse)));
        result.push_back(Pair(DataKey::Course,          control.course));
        result.push_back(Pair(DataKey::CurrentPoint,    control.currentPoint));
    }

    void fromPairs(const std::vector<Pair> &source, std::vector<FlightPoint> &fPoints)
    {
        fPoints.clear();
        uint32_t lat(0), lon(0);
        uint16_t pointNumber(0), alt(0), holdRadius(0), holdTime(0);

        for (const Pair &value: source)
        {
            switch (value.key)
            {
            case DataKey::LatitudeLowByte:
                lat |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::LatitudeHighByte:
                lat |= (value.value) & 0x0000ffff;
                break;
            case DataKey::LongitudeLowByte:
                lon |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::LongitudeHighByte:
                lon |= (value.value) & 0x0000ffff;
                break;
            case DataKey::PointNumber:
                pointNumber = value.value;
                break;
            case DataKey::Altitude:
                alt = value.value;
                break;
            case DataKey::HoldRadius:
                holdRadius = value.value;
                break;
            case DataKey::HoldTime:
                holdTime = value.value;
                break;
            case DataKey::Separator:
                if (value.value == 0xffff)
                {
                    FlightPoint point;
                    point.point.num = pointNumber;
                    point.point.lat = static_cast<double>(static_cast<int32_t>(lat)) / 10000000.;
                    point.point.lon = static_cast<double>(static_cast<int32_t>(lon)) / 10000000.;
                    point.point.alt = static_cast<float>(static_cast<int16_t>(alt));
                    point.holdTime = holdTime;
                    point.holdRadius = holdRadius;
                    lat = 0;
                    lon = 0;
                    alt = 0;
                    holdTime = 0;
                    holdRadius = 0;
                    pointNumber = 0;
                    fPoints.push_back(point);
                }
                break;
            default: break;
            }
        }
    }

    void fromPairs(const std::vector<Pair> &source, std::vector<Coords> &points)
    {
        points.clear();
        uint32_t lat(0), lon(0);
        uint16_t pointNumber(0), alt(0);

        for (const Pair &value: source)
        {
            switch (value.key)
            {
            case DataKey::PointNumber:
                pointNumber = value.value;
                break;
            case DataKey::LatitudeLowByte:
                lat |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::LatitudeHighByte:
                lat |= (value.value) & 0x0000ffff;
                break;
            case DataKey::LongitudeLowByte:
                lon |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::LongitudeHighByte:
                lon |= (value.value) & 0x0000ffff;
                break;
            case DataKey::Altitude:
                alt = value.value;
                break;
            case DataKey::Separator:
                if(value.value == 0xffff)
                {
                    Coords point;
                    point.lat = static_cast<double>(static_cast<int32_t>(lat)) / 10000000.;
                    point.lon = static_cast<double>(static_cast<int32_t>(lon)) / 10000000.;
                    point.num = pointNumber;
                    point.alt = static_cast<float>(static_cast<int16_t>(alt));
                    lat = 0;
                    lon = 0;
                    alt = 0;
                    pointNumber = 0;
                    points.push_back(point);
                }
                break;
            default: break;
            }
        }
    }

    void fromPairs(const std::vector<Pair> &source, AreaAfs &area)
    {
        area.altitude = 0;
        area.crossOverlap = 0;
        area.alongOverlap = 0;
        area.resolution = 0;
        area.points.clear();

        uint32_t lat(0), lon(0);
        uint16_t pointNumber(0);

        for (const Pair &value: source)
        {
            switch(value.key)
            {
            case DataKey::LatitudeLowByte:
                lat |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::LatitudeHighByte:
                lat |= (value.value) & 0x0000ffff;
                break;
            case DataKey::LongitudeLowByte:
                lon |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::LongitudeHighByte:
                lon |= (value.value) & 0x0000ffff;
                break;
            case DataKey::PointNumber:
                pointNumber = value.value;
                break;
            case DataKey::Altitude:
                area.altitude = static_cast<float>(static_cast<int16_t>(value.value));
                break;
            case DataKey::CrossOverlap:
                area.crossOverlap = value.value;
                break;
            case DataKey::AlongOverlap:
                area.alongOverlap = value.value;
                break;
            case DataKey::Resolution:
                area.resolution = value.value;
                break;
            case DataKey::Separator:
                if (value.value == 0xffff)
                {
                    Coords point;
                    point.lat = static_cast<double>(static_cast<int32_t>(lat)) / 10000000.;
                    point.lon = static_cast<double>(static_cast<int32_t>(lon)) / 10000000.;
                    point.num = pointNumber;
                    point.alt = area.altitude;
                    lat = 0;
                    lon = 0;
                    pointNumber = 0;
                    area.points.push_back(point);
                }
                break;
            default: break;
            }
        }
    }

    void fromPairs(const std::vector<Pair> &source, AreaRln &area)
    {
        area.altitude = 0;
        area.overlap = 0;
        area.distance = 0;
        area.format = 0;
        area.points.clear();

        uint32_t lat(0), lon(0);
        uint16_t pointNumber(0);

        for (const Pair &value: source)
        {
            switch(value.key)
            {
            case DataKey::LatitudeLowByte:
                lat |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::LatitudeHighByte:
                lat |= (value.value) & 0x0000ffff;
                break;
            case DataKey::LongitudeLowByte:
                lon |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::LongitudeHighByte:
                lon |= (value.value) & 0x0000ffff;
                break;
            case DataKey::PointNumber:
                pointNumber = value.value;
                break;
            case DataKey::Altitude:
                area.altitude = static_cast<float>(static_cast<int16_t>(value.value));
                break;
            case DataKey::Overlap:
                area.overlap = static_cast<uint8_t>(value.value);
                break;
            case DataKey::FlightDistance:
                area.distance = value.value;
                break;
            case DataKey::DataFormat:
                area.format = static_cast<uint8_t>(value.value);
                break;
            case DataKey::Separator:
                if (value.value == 0xffff)
                {
                    Coords point;
                    point.lat = static_cast<double>(static_cast<int32_t>(lat)) / 10000000.;
                    point.lon = static_cast<double>(static_cast<int32_t>(lon)) / 10000000.;
                    point.num = pointNumber;
                    point.alt = area.altitude;
                    lat = 0;
                    lon = 0;
                    pointNumber = 0;
                    area.points.push_back(point);
                }
                break;
            default: break;
            }
        }
    }

    void fromPairs(const std::vector<Pair> &source, ShootPoint &point)
    {
        point.point.lat = 0.;
        point.point.lon = 0.;
        point.point.alt = 0.f;
        point.point.num = 0;
        point.focalLength = 0;
        point.flyAround = 0;
        uint32_t lat(0), lon(0);

        for (const Pair &value: source)
        {
            switch(value.key)
            {
            case DataKey::LatitudeLowByte:
                lat |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::LatitudeHighByte:
                lat |= (value.value) & 0x0000ffff;
                break;
            case DataKey::LongitudeLowByte:
                lon |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::LongitudeHighByte:
                lon |= (value.value) & 0x0000ffff;
                break;
            case DataKey::Altitude:
                point.point.alt = static_cast<float>(static_cast<int16_t>(value.value));
                break;
            case DataKey::FocalLength:
                point.focalLength = value.value;
                break;
            case DataKey::FlyAroundFlag:
                point.flyAround = value.value;
                break;
            default: break;
            }
        }

        point.point.lat = static_cast<double>(static_cast<int32_t>(lat)) / 10000000.;
        point.point.lon = static_cast<double>(static_cast<int32_t>(lon)) / 10000000.;
    }

    void fromPairs(const std::vector<Pair> &source, TrackerEnable &result)
    {
        result.enable = false;
        result.x = 0;
        result.y = 0;

        for (const Pair &value: source)
        {
            switch(value.key)
            {
            case DataKey::ImageCoordinateX:
                result.x = value.value;
                break;
            case DataKey::ImageCoordinateY:
                result.y = value.value;
                break;
            case DataKey::TrackerEnableFlag:
                result.enable = static_cast<bool>(value.value);
                break;
            default: break;
            }
        }
    }

    void fromPairs(const std::vector<Pair> &source, GroupMode &result)
    {
        result.mode = GroupModeKey::Disabled;
        result.master = false;
        result.distancingX = 0;
        result.distancingY = 0;
        result.distancingZ = 0;

        for (const Pair &value: source)
        {
            switch(value.key)
            {
            case DataKey::GroupFlightMode:
                result.mode = static_cast<GroupModeKey>(value.value);
                break;
            case DataKey::MasterFlag:
                result.master = static_cast<bool>(value.value);
                break;
            case DataKey::DistancingX:
                result.distancingX = static_cast<int16_t>(value.value);
                break;
            case DataKey::DistancingY:
                result.distancingY = static_cast<int16_t>(value.value);
                break;
            case DataKey::DistancingZ:
                result.distancingZ = static_cast<int16_t>(value.value);
                break;
            default: break;
            }
        }
    }

    void fromPairs(const std::vector<Pair> &source, SelfId &result)
    {
        result.number = 0;
        result.type = 0;

        for (const Pair &value: source)
        {
            switch(value.key)
            {
            case DataKey::NumberUAV:
                result.number = value.value;
                break;
            case DataKey::TypeCO:
                result.type = static_cast<uint8_t>(value.value);
                break;
            default: break;
            }
        }
    }

    void fromPairs(const std::vector<Pair> &source, Telemetry &result)
    {
        result.lat = 0.;
        result.lon = 0.;
        result.alt = 0.f;
        result.pitch = 0.f;
        result.roll = 0.f;
        result.course = 0.f;
        result.speed = 0.f;

        result.flightTimeLeft = 0;
        result.groupFlightStatus = 0;
        result.dateTime = 0;
        result.boardStatus = 0;
        result.currentPoint = 0;

        uint32_t lat(0), lon(0), dateTime(0);

        for (const Pair &value: source)
        {
            switch(value.key)
            {
            case DataKey::LatitudeLowByte:
                lat |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::LatitudeHighByte:
                lat |= (value.value) & 0x0000ffff;
                break;
            case DataKey::LongitudeLowByte:
                lon |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::LongitudeHighByte:
                lon |= (value.value) & 0x0000ffff;
                break;
            case DataKey::AltitudeGPS:
                result.alt = static_cast<float>(static_cast<int16_t>(value.value));
                break;
            case DataKey::Pitch:
                result.pitch = static_cast<float>(static_cast<int16_t>(value.value)) / 10.f;
                break;
            case DataKey::Roll:
                result.roll = static_cast<float>(static_cast<int16_t>(value.value)) / 10.f;
                break;
            case DataKey::Course:
                result.course = static_cast<float>(static_cast<int16_t>(value.value)) / 10.f;
                break;
            case DataKey::Speed:
                result.speed = static_cast<float>(static_cast<int16_t>(value.value));
                break;
            case DataKey::FlightTimeLeft:
                result.flightTimeLeft = value.value;
                break;
            case DataKey::GroupFlightStatus:
                result.groupFlightStatus = static_cast<uint8_t>(value.value);
                break;
            case DataKey::TimeDateLowByte:
                dateTime |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::TimeDateHighByte:
                dateTime |= (value.value) & 0x0000ffff;
                break;
            case DataKey::BoartStatus:
                result.boardStatus = static_cast<uint8_t>(value.value);
                break;
            case DataKey::CurrentPoint:
                result.currentPoint = value.value;
                break;
            default: break;
            }
        }

        result.lat = static_cast<double>(static_cast<int32_t>(lat)) / 10000000.;
        result.lon = static_cast<double>(static_cast<int32_t>(lon)) / 10000000.;
        result.dateTime = dateTime;
    }

    void fromPairs(const std::vector<Pair> &source, NetworkParams &params)
    {
        params.portIn = 0;
        params.portOut = 0;
        params.host = 0;

        for (const Pair &value: source)
        {
            switch(value.key)
            {
            case DataKey::PortIn:
                params.portIn = value.value;
                break;
            case DataKey::PortOut:
                params.portOut = value.value;
                break;
            case DataKey::HostLowByte:
                params.host |= (value.value << 16) & 0xffff0000;
                break;
            case DataKey::HostHighByte:
                params.host |= (value.value) & 0x0000ffff;
                break;
            default: break;
            }
        }
    }

    void fromPairs(const std::vector<Pair> &source, ManualControl &control)
    {
        control.moveLeft = false;
        control.moveRight = false;
        control.moveUp = false;
        control.holdCourse = false;
        control.course = 0;
        control.currentPoint = 0;

        for (const Pair &value: source)
        {
            switch(value.key)
            {
            case DataKey::MoveLeftFlag:
                control.moveLeft = static_cast<bool>(value.value);
                break;
            case DataKey::MoveRightFlag:
                control.moveRight = static_cast<bool>(value.value);
                break;
            case DataKey::MoveUpFlag:
                control.moveUp = static_cast<bool>(value.value);
                break;
            case DataKey::HoldCourseFlag:
                control.holdCourse = static_cast<bool>(value.value);
                break;
            case DataKey::Course:
                control.course = value.value;
                break;
            case DataKey::CurrentPoint:
                control.currentPoint = value.value;
                break;
            default: break;
            }
        }
    }

    void pack(const Package &package, std::vector<char> &result)
    {
        uint64_t dataSize = k_valueSize * package.pairs.size();
        uint64_t packSize = k_headerSize + dataSize + k_crcSize;

        if (packSize >= k_maxPackageSize) return;

        result.resize(packSize);

        result[0] = k_headSymbol1;
        result[1] = k_headSymbol2;
        result[2] = k_headSymbol3;
        result[5] = static_cast<char>(package.header.source);
        result[6] = static_cast<char>(package.header.type);

        memcpy(result.data() + 3, &packSize, 2);
        memcpy(result.data() + 7, &package.header.boardNumber, 4);

        int dataStep = k_headerSize - 1;
        for (const Pair &value: package.pairs)
        {
            result[++dataStep] = static_cast<char>(value.key);
            result[++dataStep] = (0x00ff & value.value);
            result[++dataStep] = ((0xff00 & value.value) >> 8);
        }

        uint32_t crc = crc32(result.data(), packSize - k_crcSize);
        memcpy(result.data() + packSize - k_crcSize, &crc, k_crcSize);
    }

    UnpackStatus unpack(const std::vector<char> &source, Package &result)
    {
        size_t shift = 0;
        return unpack(source.data(), source.size(), shift, result);
    }

    UnpackStatus unpack(const char *source, size_t size, size_t &shift, Package &result)
    {
        result.header.source = DataSource::Unknown;
        result.header.type = DataType::Unknown;
        result.header.boardNumber = 0;
        result.pairs.clear();
        const char *shSource = source + shift;

        if (size < k_minPackageSize) { shift = size; return UnpackStatus::SmallPackageSize; }

        if (shSource[0] != k_headSymbol1  ||
            shSource[1] != k_headSymbol2  ||
            shSource[2] != k_headSymbol3)
        {
            shift = size;
            return UnpackStatus::WrongHeaderId;
        }

        uint16_t packSize = (shSource[3] & 0x00ff) | ((shSource[4] << 8) & 0xff00);
        if (size < packSize) { shift = size; return UnpackStatus::SmallPackageSize; }

        uint32_t crc = 0;
        memcpy(&crc, shSource + packSize - k_crcSize, k_crcSize);

        if (crc != crc32(shSource, packSize - k_crcSize))
            { shift = size; return UnpackStatus::WrongCrc; }

        result.header.source = static_cast<DataSource>(shSource[5]);
        result.header.type = static_cast<DataType>(shSource[6]);
        memcpy(&result.header.boardNumber, shSource + 7, 4);

        uint16_t dataSize = packSize - k_headerSize - k_crcSize;
        result.pairs.reserve(dataSize / 3);

        for (int i = k_headerSize; i < k_headerSize + dataSize; i += 3)
        {
            Pair value;
            value.key = static_cast<DataKey>(shSource[i]);
            value.value = (shSource[i + 1] & 0x00ff) | ((shSource[i + 2] << 8) & 0xff00);
            result.pairs.push_back(value);
        }

        shift = packSize + shift;
        return UnpackStatus::Success;
    }

    #endif // GF_PARSER_CPP
} // namespace GroupFlight
