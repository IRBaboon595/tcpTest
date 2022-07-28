#include <cstdint>
#include <vector>

namespace GroupFlight
{

#ifndef GF_PROTOCOL_H
#define GF_PROTOCOL_H

    //! Список устройств с которыми возможно взаимодействие
    enum class Device : uint8_t
    {
        Autopilot,  //!< Плата автопилота борта
        Computer,   //!< Вычислитель
        NPU,        //!< Наземный пункт управления
        MGP,        //!< Модули группового полета
        AFS,        //!< Камера аэро-фото съемки
        GOES_VD,    //!< Гиро-стабилизированная камера 'ВД'
        GOES_IK,    //!< Гиро-стабилизированная камера 'ИК'
        RLN         //!< РЛН
    };

    enum class PayloadType : uint8_t
    {
        AFS,        //!< Камера аэро-фото съемки
        GOES_VD,    //!< Гиро-стабилизированная камера 'ВД'
        GOES_IK,    //!< Гиро-стабилизированная камера 'ИК'
        RLN         //!< РЛН
    };

    //! Тип источника данных
    enum class DataSource : uint8_t
    {
        NPU = 0,        //!< Наземный пункт управления
        MGP = 2,        //!< Модуль группового полета
        Computer = 3,   //!< Вычислитель
        Unknown         //!< Неизвестный источник данных
    };

    //! Тип данных (тип параметрической информации)
    enum class DataType : uint8_t
    {
        // Источник данных: 3 – Вычислитель:
        SelfId = 0,                 //!< Команда 0. Самоидентификация
        FlightByPoints = 1,         //!< Команда 1. Полет по точкам, маршруту
        ReturnToHome = 2,           //!< Команда 2. Возврат на базу (в Дом)
        AreaInspectionAFS = 3,      //!< Команда 3. Обследование области АФС
        AreaInspectionRLN = 4,      //!< Команда 4. Обследование области РЛН
        CameraControlGOES = 5,      //!< Команда 5. Съемка ГОЭС. Управление камерой ГОЭС
        StartTrackerGOES = 6,       //!< Команда 6. Запуск трекера ГОЭС
        RoutePointsRequest = 7,     //!< Команда 7. Запрос точек маршрута
        GroupFlightCommand = 8,     //!< Команда 8. Команда группового полёта
        HoldPoint = 10,             //!< Команда 9. Удержание точки

        // Источник данных: 2 – МГП:
        RoutePointsResponse = 7,    //!< Команда 7. Ответ на запрос точек маршрута
        Telemetry = 9,              //!< Команда 9. Телеметрия

        // Дополнительные команды не входящие в протокол
        NetworkParams = 130,        //!< Настройка сетевого подулючения
        ManualControl = 131,        //!< Ручное управление
        ChangeSpeed = 132,          //!< Изменение скорости
        Unknown                     //!< Неизвестная команда
    };

    enum class DataKey : uint8_t
    {
        // === Источник данных: 3 – Вычислитель: ===

        //! Ключи команды 1. Полет по точкам, маршруту.
        PointNumber = 0,            //!< Номер точки
        LatitudeLowByte = 1,        //!< Широта WGS84 младшие байты
        LatitudeHighByte = 2,       //!< Широта WGS84 старшие байты
        LongitudeLowByte = 3,       //!< Долгота WGS84 младшие байты
        LongitudeHighByte = 4,      //!< Долгота WGS84 старшие байты
        Altitude = 5,               //!< Высота относительно старта, метры
        HoldRadius = 6,             //!< Радиус круга, метры
        HoldTime = 7,               //!< Время удержания, секунды

        //! Ключи команды 3. Обследование области АФС
        Resolution = 6,             //!< Разрешение, сантиметров на пиксель
        CrossOverlap = 7,           //!< Поперечное перекрытие, %
        AlongOverlap = 8,           //!< Продольное перекрытие, %

        //! Ключи команды 4. Обследование области РЛН
        AltitudeAboveSurface  = 5,  //!< Высота над поверхностью, метры
        Overlap = 6,                //!< Перекрытие, %
        FlightDistance = 7,         //!< Расстояние пролета БЛА, метры
        DataFormat = 8,             //!< //!< Формат вывода информации (0-RAW, 1–JPEG)

        //! Ключи команды 5. Съемка ГОЭС. Управление камерой ГОЭС
        AltitudeAboveSurfaceAboveSea = 5,   //!< Высота, относительно уровня моря, метры
        FocalLength = 6,                    //!< Фокусное расстояние, мм
        FlyAroundFlag = 7,                  //!< Флаг сопровождения (автоматический полет вокруг точки)


        //! Ключи команды 6. Запуск трекера ГОЭС
        ImageCoordinateX = 0,       //!< X координата картинки (горизонт)
        ImageCoordinateY = 1,       //!< Y координата картинки (вертикаль)
        TrackerEnableFlag = 2,      //!< Флаг включения трекера

        //! Ключи команды 7. Запрос точек маршрута
        RequestFlag = 0,    //!< Флаг запроса

        //! Ключи команды 8. Команда группового полёта
        GroupFlightMode = 0,        //!< Режим группового полета
        MasterFlag = 1,             //!< Флаг ведущего (0-ведущий, 1-ведомый)
        DistancingX = 2,            //!< Удаление по оси X (продольная), м
        DistancingY = 3,            //!< Удаление по оси Y (вертикальная), м
        DistancingZ = 4,            //!< Удаление по оси Z (поперечная), м

        // === Источник данных: 2 – МГП: ===

        //! Ключи команды 0. Ответ на запрос самоидентификации.
        NumberUAV = 0,              //!< Номер БЛА
        TypeCO = 1,                 //!< Тип ЦО

        //! Ключи команды 7. Ответ на запрос точек маршрута
        PointsCount = 10,           //!< Количество точек в пакете

        //! Ключи команды 9. Телеметрия
        AltitudeGPS = 5,            //!< Высота GPS, метры
        Pitch = 6,                  //!< Тангаж, град
        Roll = 7,                   //!< Крен, град
        Course = 8,                 //!< Курс, град
        FlightTimeLeft = 9,         //!< Оставшееся время полета, мин
        GroupFlightStatus = 10,     //!< Статус группового полёта борта
        TimeDateLowByte = 11,       //!< Время-Дата, мл. байты
        TimeDateHighByte = 12,      //!< Время-Дата, ст. байты
        BoartStatus = 13,           //!< Статус борта
        CurrentPoint = 14,          //!< Номер текущей точки
        Speed = 25,                 //!< Скорость

        Separator = 255,            //!< Разделитель
        Error = 255,                //!< Код ошибки

        //! Дополнительные команды не входящие в протокол

        //! Настройки сетевого подключения
        PortIn = 130,           //!< Порт для входящих сообщений
        PortOut = 131,          //!< Порт для исходящих сообщений
        HostLowByte = 132,      //!< IP-адрес младшие байты
        HostHighByte = 133,     //!< IP-адрес старшые байты

        //! Ручное управление автопилотом
        MoveLeftFlag = 134,     //!< Движение влево
        MoveRightFlag = 135,   //!< Движение вправо
        MoveUpFlag = 136,      //!< Движение вверх
        HoldCourseFlag = 137    //!< Удержание курса
    };

    //! Режим группового полёта борта
    enum class GroupModeKey : uint8_t
    {
        Disabled  = 0,      //!< отключить групповой полет
        Enabled = 1,        //!< перейти в режим группового полета
        Pause = 2,          //!< перейти в режим «Пауза» (для одного БЛА)
        GroupPauseOn = 3,   //!< перейти в режим «Пауза для группы»
        GroupPauseOff = 4   //!< выйти из «Паузы для группы»
    };

    //! Статистика группового полёта
    enum class GroupStatsKey : uint8_t
    {
        DistanceStat = 0,
        AltStat = 1,
        AngleStat = 2,
        ThrottleStat = 3,
        CourseChangeStat = 4
    };

    //! Статус группового полёта борта
    enum class GroupModeStatus : uint8_t
    {
        NotInvolved  = 0,   //!< Не участвует в группе
        MasterMode = 1,     //!< Задействован в качестве ведущего
        SlaveMode = 2       //!< Задействован в качестве ведомого
    };

    enum class ErrorType : uint8_t
    {
        NoError = 0,                //!< Ошибок нет
        BrokenPackage = 101,        //!< Битый пакет
        WrongParameters = 102,      //!< Неверные параметры
        StartupError = 103,         //!< Ошибка запуска
        SpeedLowThreshold = 128,    //!< Ошибка телеметрии -> скорость ниже порога
        SpeedHighThreshold = 129,   //!< Ошибка телеметрии -> скорость выше порога
        PitchThresholdErr = 130,    //!< Ошибка телеметрии -> недопустимый тангаж
        RollThresholdErr = 131,     //!< Ошибка телеметрии -> недопустимый крен
        CourseThresholdErr = 132,   //!< Ошибка телеметрии -> недопустимый курс
        RouteAlgortihmErr = 133     //!< Ошибка алгоритма -> m_route опережает АП
    };

    //! Заголовок каждого пакета содержит данные поля
    struct Header
    {
        DataSource source;      //!< Источник данных
        DataType type;          //!< Тип данных
        uint32_t boardNumber;   //!< Номер борта

        Header(DataSource _source, DataType _type, uint32_t _boardNumber):
            source(_source), type(_type), boardNumber(_boardNumber){}

        Header(): Header(DataSource::Unknown, DataType::SelfId, 0){}
    };

    //! Поле данных представляет из себя последовательность пар "Ключ-Значение"
    struct Pair
    {
        DataKey key;
        uint16_t value;

        Pair(DataKey _key, uint16_t _value): key(_key), value(_value){}
        Pair(): Pair(DataKey::Error, 0){}
    };

    //! Пакет состоит из заголовка и последовательности пар "Ключ-Значение"
    struct Package
    {
        Header header;
        std::vector<Pair> pairs;

        Package(const Header &_header, const std::vector<Pair> &_values):
            header(_header), pairs(_values){}

        Package(const Header &_header, const Pair &pair):
            header(_header){pairs.push_back(pair);}

        explicit Package(const Header &_header): header(_header){}

        Package(){}
    };

    // === Источник данных: 3 – Вычислитель. ===

    struct Coords
    {
        uint16_t num;       //!< Номер точки
        double  lat;        //!< Широта WGS84
        double  lon;        //!< Долгота WGS84
        float   alt = 0.f;  //!< Высота относительно старта, метры

        Coords(double _lat, double _lon, float _alt):
            num(0), lat(_lat), lon(_lon), alt(_alt){}

        Coords(uint16_t _num, double _lat, double _lon, float _alt):
            num(_num), lat(_lat), lon(_lon), alt(_alt){}

        Coords(): Coords(0., 0., 0.f){}
    };

    //! Команда 1. Полет по точкам, маршруту.
    struct FlightPoint
    {
        Coords point;               //!< Точка маршрута
        uint16_t holdRadius = 0;    //!< Радиус круга, метры
        uint16_t holdTime = 0;      //!< Время удержания, секунды

        explicit FlightPoint(const Coords &_point, uint16_t _holdRadius = 0, uint16_t _holdTime = 0):
            point(_point), holdRadius(_holdRadius), holdTime(_holdTime){}

        FlightPoint(double _lat, double _lon, float _alt,
                    uint16_t _holdRadius = 0, uint16_t _holdTime = 0):
            FlightPoint(Coords(_lat, _lon, _alt), _holdRadius, _holdTime){}

        FlightPoint(uint16_t _num, double _lat, double _lon, float _alt,
                    uint16_t _holdRadius = 0, uint16_t _holdTime = 0):
            FlightPoint(Coords(_num, _lat, _lon, _alt), _holdRadius, _holdTime){}

        FlightPoint(): FlightPoint(Coords()){}
    };

    //! Команда 3. Обследование области АФС
    struct AreaAfs
    {
        std::vector<Coords> points;     //!< Точки области
        float   altitude = 0.f;         //!< Высота, метры
        uint16_t crossOverlap = 0;      //!< Поперечное перекрытие, %
        uint16_t alongOverlap = 0;      //!< Продольное перекрытие, %
        uint16_t resolution = 0;        //!< Разрешение, сантиметров на пиксель

        AreaAfs(const std::vector<Coords> &_points,
                   float _altitude, uint16_t _crossOverlap,
                   uint16_t _alongOverlap, uint16_t _resolution = 0):
            points(_points), altitude(_altitude), crossOverlap(_crossOverlap),
            alongOverlap(_alongOverlap), resolution(_resolution){}

        AreaAfs(): AreaAfs(std::vector<Coords>(), 0.f, 0, 0, 0){}
    };

    //! Команда 3. Обследование области РЛН
    struct AreaRln
    {
        std::vector<Coords> points;     //!< Точки области
        float   altitude = 0.f;         //!< Высота, метры
        uint8_t overlap = 0;           //!< Перекрытие, %
        uint16_t distance = 0;          //!< Расстояние пролета БЛА, метры
        uint8_t format = 0;             //!< Формат вывода информации (0-RAW, 1–JPEG)

        AreaRln(const std::vector<Coords> &_points,
                   float _altitude, uint8_t _overlap,
                   uint16_t _distance, uint8_t _format):
            points(_points), altitude(_altitude), overlap(_overlap),
            distance(_distance), format(_format){}

        AreaRln(): AreaRln(std::vector<Coords>(), 0.f, 0, 0, 0){}
    };

    //! Команда 5. Съемка ГОЭС. Управление камерой ГОЭС.
    struct ShootPoint
    {
        Coords point;               //!< Координаты точки съемки
        uint16_t focalLength;       //!< Фокусное расстояние, мм
        uint16_t flyAround = 0;     //!< Радиус круга облета вокруг точки (если == 0 то, не облетать)

        ShootPoint(const Coords &_point, uint16_t _focalLength, uint16_t _flyAround = 0):
            point(_point), focalLength(_focalLength), flyAround(_flyAround){}

        ShootPoint(): ShootPoint(Coords(), 0){}
    };

    //! Команда 6. Запуск трекера ГОЭС.
    struct TrackerEnable
    {
        uint16_t x, y;   //!< X, Y координата картинки (горизонт, вертикаль)
        bool enable;     //!< Флаг включения трекера

        TrackerEnable(uint16_t _x, uint16_t _y, bool _enable):
            x(_x), y(_y), enable(_enable){}

        TrackerEnable(): TrackerEnable(0, 0, false){}
    };

    //! Команда 8. Команда группового полёта.
    struct GroupMode
    {
        GroupModeKey mode;       //!< Режим группового полета
        bool master;             //!< Флаг ведущего
        int16_t distancingX;    //!< Удаление по оси X (продольная), м
        int16_t distancingY;    //!< Удаление по оси Y (вертикальная), м
        int16_t distancingZ;    //!< Удаление по оси Z (поперечная), м

        GroupMode(GroupModeKey _mode, bool _master, int16_t _distancingX
                    , int16_t _distancingY, int16_t _distancingZ):
            mode(_mode), master(_master), distancingX(_distancingX),
            distancingY(_distancingY), distancingZ(_distancingZ) {}

        GroupMode(): GroupMode(GroupModeKey::Disabled, false, 0, 0, 0){}
    };

    // === Источник данных: 2 – МГП. ===

    //! Команда 0. Ответ на запрос самоидентификации.
    struct SelfId
    {
        uint16_t number;     //!< Номер БЛА
        uint8_t  type;       //!< Тип ЦО

        SelfId(uint16_t _number, uint8_t _type):
            number(_number), type(_type){}

        SelfId(): SelfId(0, 0){}
    };

    //! Команда 9. Телеметрия
    struct Telemetry
    {
        double  lat;        //!< Широта WGS84
        double  lon;        //!< Долгота WGS84
        float   alt;        //!< Высота GPS, метры
        float   pitch;      //!< Тангаж, град
        float   roll;       //!< Крен, град
        float   course;     //!< Курс, град
        float   speed;      //!< Скорость, м/с

        uint16_t   flightTimeLeft;       //!< Оставшееся время полета, мин
        uint8_t    groupFlightStatus;    //!< Статус группового полёта борта
        uint32_t   dateTime;             //!< Время-Дата – в Unix time
        uint8_t    boardStatus;          //!< Статус борта
        uint16_t   currentPoint;         //!< Номер текущей точки
    };

    // Дополнительные команды не входящие в протокол

    // Настройки сетевого подключения
    struct NetworkParams
    {
        uint16_t portIn;    //!< Порт для входящих сообщений
        uint16_t portOut;   //!< Порт для исходящих сообщений
        uint32_t host;      //!< IP-адрес
    };

    // Ручное управление автопилотом
    struct ManualControl
    {
        bool moveLeft = false;    //!< Движение влево
        bool moveRight = false;   //!< Движение вправо
        bool moveUp = false;      //!< Движение вверх
        bool holdCourse = false;    //!< Удержание курса
        uint16_t course = 0;         //!< Курс, град
        uint16_t currentPoint = 0;   //!< Номер текущей точки
    };


#endif // GF_PROTOCOL_H

} // namespace GroupFlight
