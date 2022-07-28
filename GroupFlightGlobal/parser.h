#include "protocol.h"

namespace GroupFlight
{

#ifndef GF_PARSER_H
#define GF_PARSER_H

    enum class UnpackStatus
    {
        UnknownError,
        Success,
        WrongCrc,
        WrongHeaderId,
        UnknownDataSource,
        UnknownDataType,
        SmallPackageSize,
    };

    //! Преобразование структур в последовательность пар "ключ-значение"
    void toPairs(const std::vector<FlightPoint> &fPoints, std::vector<Pair> &result);
    void toPairs(const std::vector<Coords> &points,       std::vector<Pair> &result);
    void toPairs(const AreaAfs &area,                     std::vector<Pair> &result);
    void toPairs(const AreaRln &area,                     std::vector<Pair> &result);
    void toPairs(const ShootPoint &point,                 std::vector<Pair> &result);
    void toPairs(const TrackerEnable &value,              std::vector<Pair> &result);
    void toPairs(const GroupMode &groupMode,              std::vector<Pair> &result);
    void toPairs(const SelfId &selfId,                    std::vector<Pair> &result);
    void toPairs(const Telemetry &telemetry,              std::vector<Pair> &result);
    void toPairs(const NetworkParams &params,             std::vector<Pair> &result);
    void toPairs(const ManualControl &control,            std::vector<Pair> &result);

    //! Преобразование последовательностей пар "ключ-значение" в соответствующие стурктуры
    void fromPairs(const std::vector<Pair> &source, std::vector<FlightPoint> &fPoints);
    void fromPairs(const std::vector<Pair> &source, std::vector<Coords> &points);
    void fromPairs(const std::vector<Pair> &source, AreaAfs &area);
    void fromPairs(const std::vector<Pair> &source, AreaRln &area);
    void fromPairs(const std::vector<Pair> &source, ShootPoint &point);
    void fromPairs(const std::vector<Pair> &source, TrackerEnable &value);
    void fromPairs(const std::vector<Pair> &source, GroupMode &groupMode);
    void fromPairs(const std::vector<Pair> &source, SelfId &selfId);
    void fromPairs(const std::vector<Pair> &source, Telemetry &telemetry);
    void fromPairs(const std::vector<Pair> &source, NetworkParams &params);
    void fromPairs(const std::vector<Pair> &source, ManualControl &control);

    //! \brief Преобразование пакета (заголовок + пары "ключ-значение") в массив std::vector<char>
    void pack(const Package &source, std::vector<char> &result);

    //! \brief Преобразование массива std::vector<char> в пакет (заголовок + пары "ключ-значение")
    UnpackStatus unpack(const std::vector<char> &source, Package &result);

    //! \brief Преобразование массива char* в пакет (заголовок + пары "ключ-значение")
    //! \param source - массив данных для преобразования
    //! \param size - размер массива
    //! \param shift - начало следующего пакета (если в массиве несколько пакетов, функция преобразует один пакет)
    //! \param result - результат преобразования одного пакета
    UnpackStatus unpack(const char *source, size_t size, size_t &shift, Package &result);

#endif // GF_PARSER_H

} // namespace GroupFlight
