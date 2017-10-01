#pragma once
#define _USE_MATH_DEFINES
#define NOMINMAX
#include <intercept.hpp>
#include <cmath>
#include <random>
//From AB
#define DELTA_T 0.02f
#define GRAVITY 9.80665f
#define DEGREES(X) (X * 180 / M_PI)
#define ABSOLUTE_ZERO_IN_CELSIUS -273.15f
#define KELVIN(t) (t - ABSOLUTE_ZERO_IN_CELSIUS)
#define CELSIUS(t) (t + ABSOLUTE_ZERO_IN_CELSIUS)
#define EARTH_ANGULAR_SPEED 0.00007292f
#define UNIVERSAL_GAS_CONSTANT 8.314f
#define WATER_VAPOR_MOLAR_MASS 0.018016f
#define DRY_AIR_MOLAR_MASS 0.028964f
#define SPECIFIC_GAS_CONSTANT_DRY_AIR 287.058f
#define STD_AIR_DENSITY_ICAO 1.22498f
#define STD_AIR_DENSITY_ASM 1.20885f
#define BC_CONVERSION_FACTOR 0.00068418f
#define SPEED_OF_SOUND(t) (331.3 + std::sqrt(1 + t / 273.15f))


struct Bullet {
    double airFriction;
    double caliber;
    double bulletLength;
    double bulletMass;
    std::vector<float> ballisticCoefficients;
    std::vector<float> velocityBoundaries;
    r_string atmosphereModel;
    int dragModel;
    std::vector<double> muzzleVelocities;
    std::vector<double> barrelLengths;
    double stabilityFactor;
    double twistDirection;
    double transonicStabilityCoef;
    double muzzleVelocity;
    vector3 origin;
    double latitude;
    double temperature;
    double altitude;
    double humidity;
    double overcast;
    double startTime;
    double lastFrame;
    double bcDegradation;
    unsigned randSeed;
    std::default_random_engine randGenerator;
};

struct Map {
    std::vector<int> gridHeights;
    std::vector<int> gridBuildingNums;
    std::vector<int> gridSurfaceIsWater;
    int mapSize;
    int mapGrids;
};

static std::vector<Bullet> bulletDatabase;
static std::unordered_map<std::string, Map> mapDatabase;
static std::string worldName = "";
static Map* map = &mapDatabase[""];


static double calculateRoughnessLength(double posX, double posY) {
    // Source: http://es.ucsc.edu/~jnoble/wind/extrap/index.html
    double roughness_lengths[10] = { 0.0002, 0.0005, 0.0024, 0.03, 0.055, 0.1, 0.2, 0.4, 0.8, 1.6 };

    int gridX = (int) floor(posX / 50);
    int gridY = (int) floor(posY / 50);
    int gridCell = gridX * map->mapGrids + gridY;

    if (gridCell >= 0 && (std::size_t)gridCell < map->gridHeights.size() && (std::size_t)gridCell < map->gridBuildingNums.size()) {
        int nearBuildings = map->gridBuildingNums[gridCell];
        int surfaceIsWater = map->gridSurfaceIsWater[gridCell];

        if (nearBuildings == 0 && surfaceIsWater == 1) {
            return 0.0005;
        }

        if (nearBuildings >= 10) {
            return 1.6;
        }

        return roughness_lengths[2 + std::min(nearBuildings, 6)];
    }

    return 0.0024;
}

static double calculateAirDensity(double temperature, double pressure, double relativeHumidity) {
    pressure = pressure * 100;

    if (relativeHumidity > 0) {
        // 610.78 gives pressure in Pa - https://en.wikipedia.org/wiki/Density_of_air
        double _pSat = 610.78 * pow(10, ((7.5 * temperature) / (temperature + 237.3)));
        double vaporPressure = relativeHumidity * _pSat;
        double partialPressure = pressure - vaporPressure;

        return (partialPressure * DRY_AIR_MOLAR_MASS + vaporPressure * WATER_VAPOR_MOLAR_MASS) / (UNIVERSAL_GAS_CONSTANT * KELVIN(temperature));
    } else {
        return pressure / (SPECIFIC_GAS_CONSTANT_DRY_AIR * KELVIN(temperature));
    }
}

static double calculateAtmosphericCorrection(double ballisticCoefficient, double temperature, double pressure, double relativeHumidity, const char *atmosphereModel) {
    double airDensity = calculateAirDensity(temperature, pressure, relativeHumidity);

    if (!strcmp(atmosphereModel, "ICAO")) {
        return (STD_AIR_DENSITY_ICAO / airDensity) * ballisticCoefficient;
    } else {
        return (STD_AIR_DENSITY_ASM / airDensity) * ballisticCoefficient;
    }
}

static double calculateRetard(int DragFunction, double DragCoefficient, double Velocity, double Mach) {
    std::vector<double> machNumbers = {};
    std::vector<double> dragCoefficients = {};
    double m = Velocity / Mach;

    // Drag Functions from: http://www.jbmballistics.com/ballistics/downloads/text/
    switch (DragFunction) {
        case 1:
            dragCoefficients = { 0.2629, 0.2558, 0.2487, 0.2413, 0.2344, 0.2278, 0.2214, 0.2155, 0.2104, 0.2061, 0.2032, 0.2020, 0.2034, 0.2165, 0.2230, 0.2313, 0.2417, 0.2546, 0.2706, 0.2901, 0.3136, 0.3415, 0.3734, 0.4084, 0.4448, 0.4805, 0.5136, 0.5427, 0.5677, 0.5883, 0.6053, 0.6191, 0.6393, 0.6518, 0.6589, 0.6621, 0.6625, 0.6607, 0.6573, 0.6528, 0.6474, 0.6413, 0.6347, 0.6280, 0.6210, 0.6141, 0.6072, 0.6003, 0.5934, 0.5867, 0.5804, 0.5743, 0.5685, 0.5630, 0.5577, 0.5527, 0.5481, 0.5438, 0.5397, 0.5325, 0.5264, 0.5211, 0.5168, 0.5133, 0.5105, 0.5084, 0.5067, 0.5054, 0.5040, 0.5030, 0.5022, 0.5016, 0.5010, 0.5006, 0.4998, 0.4995, 0.4992, 0.4990, 0.4988 };
            machNumbers = { 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50, 0.55, 0.60, 0.70, 0.725, 0.75, 0.775, 0.80, 0.825, 0.85, 0.875, 0.90, 0.925, 0.95, 0.975, 1.0, 1.025, 1.05, 1.075, 1.10, 1.125, 1.15, 1.20, 1.25, 1.30, 1.35, 1.40, 1.45, 1.50, 1.55, 1.60, 1.65, 1.70, 1.75, 1.80, 1.85, 1.90, 1.95, 2.00, 2.05, 2.10, 2.15, 2.20, 2.25, 2.30, 2.35, 2.40, 2.45, 2.50, 2.60, 2.70, 2.80, 2.90, 3.00, 3.10, 3.20, 3.30, 3.40, 3.50, 3.60, 3.70, 3.80, 3.90, 4.00, 4.20, 4.40, 4.60, 4.80, 5.00 };
            break;

        case 2:
            dragCoefficients = { 0.2303, 0.2298, 0.2287, 0.2271, 0.2251, 0.2227, 0.2196, 0.2156, 0.2107, 0.2048, 0.1980, 0.1905, 0.1828, 0.1758, 0.1702, 0.1669, 0.1664, 0.1667, 0.1682, 0.1711, 0.1761, 0.1831, 0.2004, 0.2589, 0.3492, 0.3983, 0.4075, 0.4103, 0.4114, 0.4106, 0.4089, 0.4068, 0.4046, 0.4021, 0.3966, 0.3904, 0.3835, 0.3759, 0.3678, 0.3594, 0.3512, 0.3432, 0.3356, 0.3282, 0.3213, 0.3149, 0.3089, 0.3033, 0.2982, 0.2933, 0.2889, 0.2846, 0.2806, 0.2768, 0.2731, 0.2696, 0.2663, 0.2632, 0.2602, 0.2572, 0.2543, 0.2515, 0.2487, 0.2460, 0.2433, 0.2408, 0.2382, 0.2357, 0.2333, 0.2309, 0.2262, 0.2217, 0.2173, 0.2132, 0.2091, 0.2052, 0.2014, 0.1978, 0.1944, 0.1912, 0.1851, 0.1794, 0.1741, 0.1693, 0.1648 };
            machNumbers = { 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50, 0.55, 0.60, 0.65, 0.70, 0.75, 0.775, 0.80, 0.825, 0.85, 0.875, 0.90, 0.925, 0.95, 0.975, 1.0, 1.025, 1.05, 1.075, 1.10, 1.125, 1.15, 1.175, 1.20, 1.25, 1.30, 1.35, 1.40, 1.45, 1.50, 1.55, 1.60, 1.65, 1.70, 1.75, 1.80, 1.85, 1.90, 1.95, 2.00, 2.05, 2.10, 2.15, 2.20, 2.25, 2.30, 2.35, 2.40, 2.45, 2.50, 2.55, 2.60, 2.65, 2.70, 2.75, 2.80, 2.85, 2.90, 2.95, 3.00, 3.10, 3.20, 3.30, 3.40, 3.50, 3.60, 3.70, 3.80, 3.90, 4.00, 4.20, 4.40, 4.60, 4.80, 5.00 };
            break;

        case 5:
            dragCoefficients = { 0.1710, 0.1719, 0.1727, 0.1732, 0.1734, 0.1730, 0.1718, 0.1696, 0.1668, 0.1637, 0.1603, 0.1566, 0.1529, 0.1497, 0.1473, 0.1463, 0.1489, 0.1583, 0.1672, 0.1815, 0.2051, 0.2413, 0.2884, 0.3379, 0.3785, 0.4032, 0.4147, 0.4201, 0.4278, 0.4338, 0.4373, 0.4392, 0.4403, 0.4406, 0.4401, 0.4386, 0.4362, 0.4328, 0.4286, 0.4237, 0.4182, 0.4121, 0.4057, 0.3991, 0.3926, 0.3861, 0.3800, 0.3741, 0.3684, 0.3630, 0.3578, 0.3529, 0.3481, 0.3435, 0.3391, 0.3349, 0.3269, 0.3194, 0.3125, 0.3060, 0.2999, 0.2942, 0.2889, 0.2838, 0.2790, 0.2745, 0.2703, 0.2662, 0.2624, 0.2588, 0.2553, 0.2488, 0.2429, 0.2376, 0.2326, 0.2280 };
            machNumbers = { 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50, 0.55, 0.60, 0.65, 0.70, 0.75, 0.80, 0.85, 0.875, 0.90, 0.925, 0.95, 0.975, 1.0, 1.025, 1.05, 1.075, 1.10, 1.15, 1.20, 1.25, 1.30, 1.35, 1.40, 1.45, 1.50, 1.55, 1.60, 1.65, 1.70, 1.75, 1.80, 1.85, 1.90, 1.95, 2.00, 2.05, 2.10, 2.15, 2.20, 2.25, 2.30, 2.35, 2.40, 2.45, 2.50, 2.60, 2.70, 2.80, 2.90, 3.00, 3.10, 3.20, 3.30, 3.40, 3.50, 3.60, 3.70, 3.80, 3.90, 4.00, 4.20, 4.40, 4.60, 4.80, 5.00 };
            break;

        case 6:
            dragCoefficients = { 0.2617, 0.2553, 0.2491, 0.2432, 0.2376, 0.2324, 0.2278, 0.2238, 0.2205, 0.2177, 0.2155, 0.2138, 0.2126, 0.2121, 0.2122, 0.2132, 0.2154, 0.2194, 0.2229, 0.2297, 0.2449, 0.2732, 0.3141, 0.3597, 0.3994, 0.4261, 0.4402, 0.4465, 0.4490, 0.4497, 0.4494, 0.4482, 0.4464, 0.4441, 0.4390, 0.4336, 0.4279, 0.4221, 0.4162, 0.4102, 0.4042, 0.3981, 0.3919, 0.3855, 0.3788, 0.3721, 0.3652, 0.3583, 0.3515, 0.3447, 0.3381, 0.3314, 0.3249, 0.3185, 0.3122, 0.3060, 0.3000, 0.2941, 0.2883, 0.2772, 0.2668, 0.2574, 0.2487, 0.2407, 0.2333, 0.2265, 0.2202, 0.2144, 0.2089, 0.2039, 0.1991, 0.1947, 0.1905, 0.1866, 0.1794, 0.1730, 0.1673, 0.1621, 0.1574 };
            machNumbers = { 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50, 0.55, 0.60, 0.65, 0.70, 0.75, 0.80, 0.85, 0.875, 0.90, 0.925, 0.95, 0.975, 1.0, 1.025, 1.05, 1.075, 1.10, 1.125, 1.15, 1.175, 1.20, 1.225, 1.25, 1.30, 1.35, 1.40, 1.45, 1.50, 1.55, 1.60, 1.65, 1.70, 1.75, 1.80, 1.85, 1.90, 1.95, 2.00, 2.05, 2.10, 2.15, 2.20, 2.25, 2.30, 2.35, 2.40, 2.45, 2.50, 2.60, 2.70, 2.80, 2.90, 3.00, 3.10, 3.20, 3.30, 3.40, 3.50, 3.60, 3.70, 3.80, 3.90, 4.00, 4.20, 4.40, 4.60, 4.80, 5.00 };
            break;

        case 7:
            dragCoefficients = { 0.1198, 0.1197, 0.1196, 0.1194, 0.1193, 0.1194, 0.1194, 0.1194, 0.1193, 0.1193, 0.1194, 0.1193, 0.1194, 0.1197, 0.1202, 0.1207, 0.1215, 0.1226, 0.1242, 0.1266, 0.1306, 0.1368, 0.1464, 0.1660, 0.2054, 0.2993, 0.3803, 0.4015, 0.4043, 0.4034, 0.4014, 0.3987, 0.3955, 0.3884, 0.3810, 0.3732, 0.3657, 0.3580, 0.3440, 0.3376, 0.3315, 0.3260, 0.3209, 0.3160, 0.3117, 0.3078, 0.3042, 0.3010, 0.2980, 0.2951, 0.2922, 0.2892, 0.2864, 0.2835, 0.2807, 0.2779, 0.2752, 0.2725, 0.2697, 0.2670, 0.2643, 0.2615, 0.2588, 0.2561, 0.2533, 0.2506, 0.2479, 0.2451, 0.2424, 0.2368, 0.2313, 0.2258, 0.2205, 0.2154, 0.2106, 0.2060, 0.2017, 0.1975, 0.1935, 0.1861, 0.1793, 0.1730, 0.1672, 0.1618 };
            machNumbers = { 0.0, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50, 0.55, 0.60, 0.65, 0.70, 0.725, 0.75, 0.775, 0.80, 0.825, 0.85, 0.875, 0.90, 0.925, 0.95, 0.975, 1.0, 1.025, 1.05, 1.075, 1.10, 1.125, 1.15, 1.20, 1.25, 1.30, 1.35, 1.40, 1.50, 1.55, 1.60, 1.65, 1.70, 1.75, 1.80, 1.85, 1.90, 1.95, 2.00, 2.05, 2.10, 2.15, 2.20, 2.25, 2.30, 2.35, 2.40, 2.45, 2.50, 2.55, 2.60, 2.65, 2.70, 2.75, 2.80, 2.85, 2.90, 2.95, 3.00, 3.10, 3.20, 3.30, 3.40, 3.50, 3.60, 3.70, 3.80, 3.90, 4.00, 4.20, 4.40, 4.60, 4.80, 5.00 };
            break;

        case 8:
            dragCoefficients = { 0.2105, 0.2105, 0.2104, 0.2104, 0.2103, 0.2103, 0.2103, 0.2103, 0.2103, 0.2102, 0.2102, 0.2102, 0.2102, 0.2102, 0.2103, 0.2103, 0.2104, 0.2104, 0.2105, 0.2106, 0.2109, 0.2183, 0.2571, 0.3358, 0.4068, 0.4378, 0.4476, 0.4493, 0.4477, 0.4450, 0.4419, 0.4353, 0.4283, 0.4208, 0.4133, 0.4059, 0.3986, 0.3915, 0.3845, 0.3777, 0.3710, 0.3645, 0.3581, 0.3519, 0.3458, 0.3400, 0.3343, 0.3288, 0.3234, 0.3182, 0.3131, 0.3081, 0.3032, 0.2983, 0.2937, 0.2891, 0.2845, 0.2802, 0.2720, 0.2642, 0.2569, 0.2499, 0.2432, 0.2368, 0.2308, 0.2251, 0.2197, 0.2147, 0.2101, 0.2058, 0.2019, 0.1983, 0.1950, 0.1890, 0.1837, 0.1791, 0.1750, 0.1713 };
            machNumbers = { 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50, 0.55, 0.60, 0.65, 0.70, 0.75, 0.80, 0.825, 0.85, 0.875, 0.90, 0.925, 0.95, 0.975, 1.0, 1.025, 1.05, 1.075, 1.10, 1.125, 1.15, 1.20, 1.25, 1.30, 1.35, 1.40, 1.45, 1.50, 1.55, 1.60, 1.65, 1.70, 1.75, 1.80, 1.85, 1.90, 1.95, 2.00, 2.05, 2.10, 2.15, 2.20, 2.25, 2.30, 2.35, 2.40, 2.45, 2.50, 2.60, 2.70, 2.80, 2.90, 3.00, 3.10, 3.20, 3.30, 3.40, 3.50, 3.60, 3.70, 3.80, 3.90, 4.00, 4.20, 4.40, 4.60, 4.80, 5.00 };
            break;

        default:
            break;

    }

    for (int i = 0; i < machNumbers.size(); i++) {
        if (machNumbers[i] >= m) {
            int previousIdx = std::max(0, i - 1);
            double previousDragCoefficient = dragCoefficients[previousIdx];
            double previousMachNumber = machNumbers[previousIdx];
            double cd = previousDragCoefficient + (dragCoefficients[i] - previousDragCoefficient) * (m - previousMachNumber) / (machNumbers[i] - previousMachNumber);
            return BC_CONVERSION_FACTOR * (cd / DragCoefficient) * std::pow(Velocity, 2);
        }
    }

    return 0.0;
}

static double calculateVanillaZeroAngle(double zeroRange, double muzzleVelocity, double airFriction, double boreHeight) {
    double zeroAngle = 0.0f;

    for (int i = 0; i < 10; i++) {
        double lx = 0.0f;
        double ly = 0.0f;

        double px = 0.0f;
        double py = -boreHeight / 100.0f;

        double gx = std::sin(zeroAngle) * -GRAVITY;
        double gy = std::cos(zeroAngle) * -GRAVITY;

        double vx = std::cos(zeroAngle) * muzzleVelocity;
        double vy = std::sin(zeroAngle) * muzzleVelocity;

        double tof = 0.0f;
        double v = 0.0f;

        while (tof < 8.0f && px < zeroRange) {
            lx = px;
            ly = py;

            v = std::sqrt(vx*vx + vy*vy);

            double ax = vx * v * airFriction;
            double ay = vy * v * airFriction;
            ax += gx;
            ay += gy;

            px += vx * DELTA_T * 0.5;
            py += vy * DELTA_T * 0.5;
            vx += ax * DELTA_T;
            vy += ay * DELTA_T;
            px += vx * DELTA_T * 0.5;
            py += vy * DELTA_T * 0.5;

            tof += DELTA_T;
        }

        double y = ly + (zeroRange - lx) * (py - ly) / (px - lx);
        double offset = -std::atan(y / zeroRange);
        zeroAngle += offset;

        if (std::abs(offset) < 0.0001f) {
            break;
        }
    }

    return zeroAngle;
}

static double calculateZeroAngle(double zeroRange, double muzzleVelocity, double boreHeight, double temperature, double pressure, double humidity, double ballisticCoefficient, int dragModel, char*  atmosphereModel) {
    double zeroAngle = 0.0f;

    ballisticCoefficient = calculateAtmosphericCorrection(ballisticCoefficient, temperature, pressure, humidity, atmosphereModel);

    for (int i = 0; i < 10; i++) {
        double lx = 0.0f;
        double ly = 0.0f;

        double px = 0.0f;
        double py = -boreHeight / 100.0f;

        double gx = std::sin(zeroAngle) * -GRAVITY;
        double gy = std::cos(zeroAngle) * -GRAVITY;

        double vx = std::cos(zeroAngle) * muzzleVelocity;
        double vy = std::sin(zeroAngle) * muzzleVelocity;

        double tof = 0.0f;
        double v = 0.0f;

        while (tof < 8.0f && px < zeroRange) {
            lx = px;
            ly = py;

            v = std::sqrt(vx*vx + vy*vy);

            double retard = calculateRetard(dragModel, ballisticCoefficient, v, SPEED_OF_SOUND(15));
            double ax = vx / v * -retard;
            double ay = vy / v * -retard;
            ax += gx;
            ay += gy;

            px += vx * DELTA_T * 0.5;
            py += vy * DELTA_T * 0.5;
            vx += ax * DELTA_T;
            vy += ay * DELTA_T;
            px += vx * DELTA_T * 0.5;
            py += vy * DELTA_T * 0.5;

            tof += DELTA_T;
        }

        double y = ly + (zeroRange - lx) * (py - ly) / (px - lx);
        double offset = -std::atan(y / zeroRange);
        zeroAngle += offset;

        if (std::abs(offset) < 0.0001f) {
            break;
        }
    }

    return zeroAngle;
}






class AB {
public:
    static game_value simulate(game_value left, game_value right);
    static game_value newBullet(game_value right);
    static game_value worldInit(game_value left, game_value right);
    static game_value worldSet(game_value right);
};
