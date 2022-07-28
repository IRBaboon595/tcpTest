#include "structs.h"
#include "protocol.h"

//! Файл описвает алгоритмы работы с гографическими координатами
//! и координатами проекции Меркатора,
//! необходимые для работы модуля "GroupFlight"

namespace GroupFlight
{
#ifndef GF_COORDS_H
#define GF_COORDS_H

constexpr double kRadian = 5.729577951308232087679815481410517033e+01;          // градусов в радиане
constexpr double kDegree = 1.745329251994329576923690768488612713e-02;          // радиан в градусе (2*pi/360)
constexpr double kEarthRadiusWGS84Major = 6378137.0;                            // радиус земли эллипсойд WGS-84(G1150) / ITRF-2000
constexpr double kEarthRadiusWGS84Mean = 6371000.0;                             // усредненный радиус земли
constexpr double kEarthEccentricityWGS84 = 0.081819190842620223569348070214;    // эксцентриситет земли эллипсойд WGS-84(G1150) / ITRF-2000

//! \brief Перевод градусы в радианы и обратно
inline constexpr double degToRad(double a) { return (a * kDegree); }
inline constexpr double radToDeg(double a) { return (a * kRadian); }
inline constexpr float degToRad(float a) { return (a * static_cast<float>(kDegree)); }
inline constexpr float radToDeg(float a) { return (a * static_cast<float>(kRadian)); }
inline Coords degToRad(Coords a) { return Coords(a.num, a.lat * kDegree, a.lon * kDegree, a.alt); }
inline Coords radToDeg(Coords a) { return Coords(a.num, a.lat * kRadian, a.lon * kRadian, a.alt); }

inline bool operator==(const Coords &p1, const Coords &p2)
{
    return ((!p1.lon || !p2.lon) ? isNullDouble(p1.lon - p2.lon) : compareDouble(p1.lon, p2.lon)) &&
           ((!p1.lat || !p2.lat) ? isNullDouble(p1.lat - p2.lat) : compareDouble(p1.lat, p2.lat)) &&
           ((!p1.alt || !p2.alt) ? isNullFloat(p1.alt - p2.alt) : compareFloat(p1.alt, p2.alt));
}

inline bool operator!=(const Coords &p1, const Coords &p2){ return !(p1 == p2);}

//! \brief Перевод координат проекции меркатора в географические

inline Coords mercToGeoCoords(const Point &mercCoords)
{
    double ts = exp(mercCoords.y / kEarthRadiusWGS84Major);
    double phi = kHalfPi - 2 * atan(ts);
    return Coords(radToDeg(phi), radToDeg(mercCoords.x) / kEarthRadiusWGS84Major, 0.f);
}

//! \brief Перевод географических координат в координаты проекции меркатора
inline Point geoToMercCoords(const Coords &geoCoords)
{
    double lat = fmin(89.5, fmax(geoCoords.lat, -89.5));
    double phi = degToRad(lat);
    double ts = tan(kHalf * (kHalfPi - phi));
    return Point(degToRad(geoCoords.lon) * kEarthRadiusWGS84Major, kEarthRadiusWGS84Major * log(ts));
}

//! \brief Расчет размера пикселя на метр на основе географических координат
inline double scenePixelsPerMeter(const Coords &geoCoords)
{
    return 1.0 / cos(degToRad(geoCoords.lon));
}

//! \brief Расчет расстояния между координатами, радианы
inline double distanceRad(const Coords &fromRad, const Coords &toRad)
{
    if (fromRad == toRad) return 0.f;

    double sin1, cos1, sin2, cos2;
    sincos(fromRad.lat, &sin1, &cos1);
    sincos(toRad.lat, &sin2, &cos2);
    double cos21 = cos(toRad.lon - fromRad.lon);
    double d1 = sin1;
    d1 *= sin2;
    double d2 = cos1;
    d2 *= cos2;
    d2 *= cos21;
    d1 += d2;
    d1 = kEarthRadiusWGS84Mean * acos(d1);

    return d1;
}
//! \brief Расчет расстояния между координатами, градусы
inline double distanceDeg(const Coords &fromDeg, const Coords &toDeg)
{
    return distanceRad(degToRad(fromDeg), degToRad(toDeg));
}

//! \brief Нормализация угла [0 : 2pi]
inline double angleNormalizeRad(double angle)
{
    if (fabs(angle) >= kTwoPi)
        angle -= static_cast<long long>(angle * kOneDivTwoPi) * kTwoPi;

    if (angle < 0.0) angle += kTwoPi;

    return angle;
}

inline double angleNormalizeDeg(double angle)
{
    if (fabs(angle) >= 360.0)
        angle -= static_cast<long long>(angle * kOneDiv360)*360LL;
    if (angle < 0.0)
        angle += 360.0;
    return angle;
}

inline double angleDiffDeg(double angle1, double angle2)
{
    const double delta = angle2 - angle1;
    return angleNormalizeDeg(delta);
}

//! \brief Разница углов [0 : 2pi]
inline double angleDiffRad(double angle1, double angle2)
{
    return angleNormalizeRad(angle2 - angle1);//(delta < 0.0 ? delta + 360.0 : delta);
}

//! \brief Разница углов [-pi : pi]
inline double angleDiffRad314(double angle1, double angle2)
{
    const double angle = angleDiffRad(angle1, angle2);
    return (angle > kPi ? angle - kTwoPi : angle);
}

inline double angleDiffDeg180(double angle1, double angle2)
{
    const double angle = angleDiffDeg(angle1, angle2);
    return (angle > 180.0 ? angle - 360.0 : angle);
}

//! \brief Азимут между координатами в радианах,  p1, p2 - радианы
//! В данной функции производиться расчёт координат Меркатора из экваториальных координат, по формулам.
//! Из полученных Меркторовских координат, выполняется расчёт азимута между ведомым бортом и ведущим в полярных параметрах.
//! Расчёт в горизонтальных координатах (Меркатор)
//! Ось Х - широты, ось У - долготы
//! Выполняется расчёт в лог-полярных координатах
inline double azimuthRad(const Coords &p1_rad, const Coords &p2_rad)
{
    if (p1_rad == p2_rad) return 0.0;

    const double sin1 = sin(p2_rad.lat) * kEarthEccentricityWGS84;  // в формулах пересчёта в меркатор учитывается эксцентриситет. См. формулу для координаты У
    const double sin2 = sin(p1_rad.lat) * kEarthEccentricityWGS84;
    const double com = kEarthEccentricityWGS84 * kHalf;
    double temp = angleDiffRad314(p2_rad.lon, p1_rad.lon);      // поправка для нахождения азимута по кратчайшему пути (19.06.2015)
    double alpha = temp  /
            ( log(tan(kQuarterPi + p1_rad.lat * kHalf) * pow(((1.0 - sin2)/(1.0 + sin2)), com))
            - log(tan(kQuarterPi + p2_rad.lat * kHalf) * pow(((1.0 - sin1)/(1.0 + sin1)), com)) );
    alpha = atan(alpha);

    return angleNormalizeRad(alpha + (p2_rad.lat > p1_rad.lat ? 0.0 : kPi));    // учитываем случай, когда разница широт отрицательная - инвертируем её
}

//! \brief Азимут между координатами в градусах,  p1, p2 - градусы
inline double azimuthDeg(const Coords &p1_deg, const Coords &p2_deg)
{
    return azimuthRad(degToRad(p1_deg), degToRad(p2_deg));
}

//! \brief Перемещение точки вдоль заданного угла
inline Coords movePosition(const Coords &pos, double azimuth, float distance)
{
    //φ2 = asin( sin(φ1)*cos(d/R) + cos(φ1)*sin(d/R)*cos(θ) )
    //λ2 = λ1 + atan2( sin(θ)*sin(d/R)*cos(φ1), cos(d/R)−sin(φ1)*sin(φ2) )
    if (distance == 0.f) return pos;

    Coords result;
    distance /= kEarthRadiusWGS84Mean;       // угловое расстояние в радианах по меркатору λ = x / EarthRad
    double sinA, cosA;
    sincos(distance, &sinA, &cosA);          // distance - угловое расстояние, а sinA и cosA - его проекции на сферу (см. сферическая теорема косинусов)
    result.lat = asin(sin(pos.lat) * cosA + cos(pos.lat) * sinA * cos(azimuth));
    result.lon = pos.lon + atan2(sin(azimuth) * sinA * cos(pos.lat), cosA - sin(pos.lat) * sin(result.lat));
    result.alt = pos.alt;

    if (result.lon > kPi) result.lon -= kTwoPi;
    return result;
}

//! \brief Расчет курса для полета по кругу
//! \param position - текущая позиция БЛА в градусах
//! \param point2 - позиция центра круга в градусах
//! \param radius - радиус круга в метрах
//! \param dirClockwise - направление движения по кругу (true - по часовой, false - против)
//! \return Требуемый курс в градусах
inline int circle(const Coords &position, const Coords &point2, int radius, bool dirClockwise)
{
    const Coords coord(degToRad(position.lat), degToRad(position.lon), position.alt);
    const Coords point(degToRad(point2.lat), degToRad(point2.lon), position.alt);
    float distToPoint = distanceRad(coord, point);
    float azimuthToPoint = radToDeg(azimuthRad(point, coord));

    float k = distToPoint / (radius); // отношение расстояния до точки к радиусу круга (для выхода на круг по спирали)
    float sectorAngle = bound(0.f, radToDeg(100.f/distToPoint) + (k < 1.f ? 60.f * (1-k) : 0.f), 60.f) ; //
    float angle = azimuthToPoint + (dirClockwise ? sectorAngle : - sectorAngle);
    angle = angleNormalizeDeg(angle);
    Coords pOnCircle = movePosition(point, degToRad(angle), radius);

    int m_course = radToDeg(azimuthRad(coord, pOnCircle));
    m_course = (m_course < 2 ? 360 : m_course);

    return m_course;
}

//! \brief Сдвиг координат на заданное расстояние
//! \param pos - координаты для сдвига
//! \param x - сдвиг по оси X в метрах
//! \param y - сдвиг по оси Y в метрах
//! \param z - сдвиг по оси Z в метрах
inline void shiftCoords(Coords &pos, int x, int y, int z = 0)
{
    pos.lat = pos.lat + (static_cast<double>(y) / kEarthRadiusWGS84Major) * (180. / kPi);
    pos.lon = pos.lon + (static_cast<double>(x) / kEarthRadiusWGS84Major) * (180. / kPi) / cos(pos.lat * kPi / 180.);
    pos.alt += z;
}

inline double cyclicToRange(double value, double range)
{
    if (fabs(value) < range) return value;

    int del = value / range;
    double result = value - (range * del);

    if (del % 2)
    {
        if (value > 0) result -= range;
        else result += range;
    }

    return result;
}

//! \brief Нормализация координат
inline Coords normalizePositionRad(const Coords &pos)
{
    double latNormalized = cyclicToRange(pos.lat, kHalfPi);
    double lonNormalized = cyclicToRange(pos.lon, kPi);
    return Coords(latNormalized, lonNormalized, pos.alt);
}

#endif // GF_COORDS_H

} // namespace GroupFlight

