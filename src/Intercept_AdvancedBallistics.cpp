#include "Intercept_AdvancedBallistics.hpp"
using namespace intercept;

game_value AB::simulate(game_value left, game_value right) {
    
    //_bullet advancedBallisticsSimulate [_index,_bulletVelocity, _bulletPosition, ACE_wind, ASLToATL(_bulletPosition) select 2, _aceTimeSecond, CBA_missionTime - _aceTimeSecond];


    // simulate:0:[-0.109985,542.529,-3.98301]:[3751.57,5332.23,214.252]:[0.598153,2.38829,0]:28.6:0:0.481542:0:215.16
    unsigned int index = 0;
    char* velocityArray;
    vector3 velocity = { 0.0, 0.0, 0.0 };
    char* positionArray;
    vector3 position = { 0.0, 0.0, 0.0 };
    char* windArray;
    double heightAGL = 0.0;
    double tickTime = 0.0;

    index = (float)right[0];

    velocity = right[1];

    position = right[2];

    vector3 wind = right[3];
    heightAGL = right[4];
    tickTime = right[5];
    tickTime += (float)right[6];

    if (bulletDatabase[index].randSeed == 0) {
        int angle = (int) round(atan2(velocity.x, velocity.y) * 360 / M_PI);
        bulletDatabase[index].randSeed = (unsigned) (720 + angle) % 720;
        bulletDatabase[index].randSeed *= 3;
        bulletDatabase[index].randSeed += (unsigned) round(abs(velocity.z) / 2);
        bulletDatabase[index].randSeed *= 3;
        bulletDatabase[index].randSeed += (unsigned) round(abs(bulletDatabase[index].origin[0] / 2));
        bulletDatabase[index].randSeed *= 3;
        bulletDatabase[index].randSeed += (unsigned) round(abs(bulletDatabase[index].origin[1] / 2));
        bulletDatabase[index].randSeed *= 3;
        bulletDatabase[index].randSeed += (unsigned) abs(bulletDatabase[index].temperature) * 10;
        bulletDatabase[index].randSeed *= 3;
        bulletDatabase[index].randSeed += (unsigned) abs(bulletDatabase[index].humidity) * 10;
        bulletDatabase[index].randGenerator.seed(bulletDatabase[index].randSeed);
    }

    double ballisticCoefficient = 1.0;
    double dragRef = 0.0;
    double drag = 0.0;
    vector3 accelRef = { 0.0, 0.0, 0.0 };
    vector3 accel = { 0.0, 0.0, 0.0 };
    double TOF = 0.0;
    double deltaT = 0.0;
    vector3 trueVelocity = { 0.0, 0.0, 0.0 };
    float trueSpeed = 0.0;
    double temperature = 0.0;
    double pressure = 1013.25;
    double windSpeed = 0.0;
    float windAttenuation = 1.0f;
    vector3 velocityOffset = { 0.0, 0.0, 0.0 };

    TOF = tickTime - bulletDatabase[index].startTime;

    deltaT = tickTime - bulletDatabase[index].lastFrame;
    bulletDatabase[index].lastFrame = tickTime;

    double bulletSpeed = velocity.magnitude();

    windSpeed = wind.magnitude();
    if (windSpeed > 0.1) {

        vector3 windSourceTerrain = position - wind / windSpeed * 100;

        int gridX = (int) floor(windSourceTerrain.x / 50);
        int gridY = (int) floor(windSourceTerrain.y / 50);
        int gridCell = gridX * map->mapGrids + gridY;

        if (gridCell >= 0 && (std::size_t)gridCell < map->gridHeights.size() && (std::size_t)gridCell < map->gridBuildingNums.size()) {
            double gridHeight = map->gridHeights[gridCell];

            if (gridHeight > position.z) {
                double angle = atan((gridHeight - position.z) / 100);
                windAttenuation *= pow(abs(cos(angle)), 2);
            }
        }
    }

    if (windSpeed > 0.1) {

        vector3 windSourceObstacles = position - wind / windSpeed * 25;

        if (heightAGL > 0 && heightAGL < 20) {
            double roughnessLength = calculateRoughnessLength(windSourceObstacles.x, windSourceObstacles.y);
            windAttenuation *= abs(log(heightAGL / roughnessLength) / log(20 / roughnessLength));
        }
    }

    if (windAttenuation < 1) {
        wind = wind * windAttenuation;
        windSpeed = wind.magnitude();
    }

    trueVelocity = velocity - wind;
    trueSpeed = trueVelocity.magnitude();

    double speedOfSound = 331.3 + (0.6 * temperature);
    double transonicSpeed = 394 + (0.6 * temperature);
    if (bulletDatabase[index].transonicStabilityCoef < 1.0f && bulletSpeed < transonicSpeed && bulletSpeed > speedOfSound) {
        std::uniform_real_distribution<double> distribution(-10.0, 10.0);
        double coef = 1.0f - bulletDatabase[index].transonicStabilityCoef;

        trueVelocity.x += distribution(bulletDatabase[index].randGenerator) * coef;
        trueVelocity.y += distribution(bulletDatabase[index].randGenerator) * coef;
        trueVelocity.z += distribution(bulletDatabase[index].randGenerator) * coef;
        float speed = trueVelocity.magnitude();

        trueVelocity = trueVelocity * trueSpeed / speed;

        bulletDatabase[index].bcDegradation *= pow(0.993, coef);
    };

    temperature = bulletDatabase[index].temperature - 0.0065 * position.z;
    pressure = (1013.25 - 10 * bulletDatabase[index].overcast) * pow(1 - (0.0065 * (bulletDatabase[index].altitude + position.z)) / (273.15 + temperature + 0.0065 * bulletDatabase[index].altitude), 5.255754495);

    if (bulletDatabase[index].ballisticCoefficients.size() == bulletDatabase[index].velocityBoundaries.size() + 1) {
        dragRef = deltaT * bulletDatabase[index].airFriction * bulletSpeed * bulletSpeed;

        accelRef = (velocity / bulletSpeed) * dragRef;

        velocityOffset = velocityOffset - accelRef;

        ballisticCoefficient = bulletDatabase[index].ballisticCoefficients[0];
        for (int i = (int) bulletDatabase[index].velocityBoundaries.size() - 1; i >= 0; i = i - 1) {
            if (bulletSpeed < bulletDatabase[index].velocityBoundaries[i]) {
                ballisticCoefficient = bulletDatabase[index].ballisticCoefficients[i + 1];                                                 
                break;
            }
        }

        ballisticCoefficient = calculateAtmosphericCorrection(ballisticCoefficient, temperature, pressure, bulletDatabase[index].humidity, bulletDatabase[index].atmosphereModel.c_str());
        ballisticCoefficient *= bulletDatabase[index].bcDegradation;
        drag = deltaT * calculateRetard(bulletDatabase[index].dragModel, ballisticCoefficient, trueSpeed, SPEED_OF_SOUND(temperature));
        accel = (trueVelocity / trueSpeed) * drag;
        velocityOffset = velocityOffset - accel;
    } else {
        double airDensity = calculateAirDensity(temperature, pressure, bulletDatabase[index].humidity);
        double airFriction = bulletDatabase[index].airFriction * airDensity / STD_AIR_DENSITY_ICAO;

        if (airFriction != bulletDatabase[index].airFriction || windSpeed > 0) {
            dragRef = deltaT * bulletDatabase[index].airFriction * bulletSpeed * bulletSpeed;

            accelRef = (velocity / bulletSpeed) * dragRef;

            velocityOffset =  velocityOffset - accelRef;

            drag = deltaT * airFriction * trueSpeed * trueSpeed;
            accel = (trueVelocity / trueSpeed) * drag;

            velocityOffset = velocityOffset + accel;
        }
    }

    if (TOF > 0) {
        float bulletDir = atan2(velocity.x, velocity.y);
        float driftAccel = bulletDatabase[index].twistDirection * (0.0482251 * (bulletDatabase[index].stabilityFactor + 1.2)) / pow(TOF, 0.17);
        double driftVelocity = 0.0581025 *(bulletDatabase[index].stabilityFactor + 1.2) * pow(TOF, 0.83);
        double dragCorrection = (driftVelocity / bulletSpeed) * drag;
        velocityOffset.x += sin(bulletDir + M_PI / 2) * (driftAccel * deltaT + dragCorrection);
        velocityOffset.y += cos(bulletDir + M_PI / 2) * (driftAccel * deltaT + dragCorrection);
    }

    double lat = bulletDatabase[index].latitude;
    accel.x = 2 * EARTH_ANGULAR_SPEED * +(velocity.y * sin(lat) - velocity.z * cos(lat));
    accel.y = 2 * EARTH_ANGULAR_SPEED * -(velocity.x * sin(lat));
    accel.z = 2 * EARTH_ANGULAR_SPEED * +(velocity.x * cos(lat));

    velocityOffset = velocityOffset + accel * deltaT;

    intercept::sqf::set_velocity(left, sqf::velocity(left)+velocityOffset);
    return {};
}

intercept::types::game_value AB::newBullet(game_value right) {

    //[GVAR(currentbulletID), _airFriction, _ballisticCoefficients, _velocityBoundaries, _atmosphereModel, _dragModel, _stabilityFactor, 
    //_twistDirection, _muzzleVelocity, _transonicStabilityCoef, 
    //index 10 ASL pos
    //getPosASL _projectile, EGVAR(common, mapLatitude), EGVAR(weather, currentTemperature), EGVAR(common, mapAltitude), EGVAR(weather, currentHumidity), overcast, _aceTimeSecond, CBA_missionTime - _aceTimeSecond];

    unsigned int index = 0;
    double airFriction = 0.0;
    char* ballisticCoefficientArray;
    char* ballisticCoefficient;
    std::vector<float> ballisticCoefficients;
    char* velocityBoundaryArray;
    char* velocityBoundary;
    std::vector<float> velocityBoundaries;
    int dragModel = 1;
    double stabilityFactor = 1.5;
    int twistDirection = 1;
    double transonicStabilityCoef = 1;
    double muzzleVelocity = 850;
    char* originArray;
    char* originEntry;
    double latitude = 0.0;
    double temperature = 0.0;
    double altitude = 0.0;
    double humidity = 0.0;
    double overcast = 0.0;
    double tickTime = 0.0;

    index = (float)right[0];
    airFriction = right[1];
    ballisticCoefficients.reserve(right[2].to_array().count());
    for (float coef : right[2].to_array()) {
        ballisticCoefficients.emplace_back(coef);
    }

    velocityBoundaries.reserve(right[3].to_array().count());
    for (float bound : right[3].to_array()) {
        velocityBoundaries.emplace_back(bound);
    }

    r_string atmosphereModel = right[4];
    dragModel = right[5];
    stabilityFactor = right[6];
    twistDirection = right[7];
    muzzleVelocity = right[8];
    transonicStabilityCoef = right[9];

    vector3 origin = right[10];

    latitude = right[11];
    temperature = right[12];
    altitude = right[13];
    humidity = right[14];
    overcast = right[15];
    tickTime = right[16];
    tickTime += (float)right[17];

    while (index >= bulletDatabase.size()) {
        Bullet bullet;
        bulletDatabase.push_back(bullet);
    }

    bulletDatabase[index].airFriction = airFriction;
    bulletDatabase[index].ballisticCoefficients = std::move(ballisticCoefficients);
    bulletDatabase[index].velocityBoundaries = std::move(velocityBoundaries);
    bulletDatabase[index].atmosphereModel = std::move(atmosphereModel);
    bulletDatabase[index].dragModel = dragModel;
    bulletDatabase[index].stabilityFactor = stabilityFactor;
    bulletDatabase[index].twistDirection = twistDirection;
    bulletDatabase[index].transonicStabilityCoef = transonicStabilityCoef;
    bulletDatabase[index].muzzleVelocity = muzzleVelocity;
    bulletDatabase[index].origin = origin;
    bulletDatabase[index].latitude = latitude / 180 * M_PI;
    bulletDatabase[index].temperature = temperature;
    bulletDatabase[index].altitude = altitude;
    bulletDatabase[index].humidity = humidity;
    bulletDatabase[index].overcast = overcast;
    bulletDatabase[index].startTime = tickTime;
    bulletDatabase[index].lastFrame = tickTime;
    bulletDatabase[index].bcDegradation = 1.0;
    bulletDatabase[index].randSeed = 0;
}

game_value AB::worldInit(game_value left, game_value right) {
    int mapSize = 0;
    int mapGrids = 0;
    unsigned int gridCells = 0;

    worldName = left;
    mapSize = right;

    mapGrids = (int) ceil((double) mapSize / 50.0) + 1;
    gridCells = mapGrids * mapGrids;

    map = &mapDatabase[worldName];
    if (map->gridHeights.size() == gridCells) {
        return true;
    }

    map->mapSize = mapSize;
    map->mapGrids = mapGrids;
    map->gridHeights.clear();
    map->gridBuildingNums.clear();
    map->gridSurfaceIsWater.clear();
    map->gridHeights.reserve(gridCells);
    map->gridBuildingNums.reserve(gridCells);
    map->gridSurfaceIsWater.reserve(gridCells);

    return false;
}
game_value AB::worldSet(game_value right) {
    
    int height = 0;
    int numObjects = 0;
    int surfaceIsWater = 0;

    height = right[0];
    numObjects = right[1];
    surfaceIsWater = right[2];

    map->gridHeights.push_back(height);
    map->gridBuildingNums.push_back(numObjects);
    map->gridSurfaceIsWater.push_back(surfaceIsWater);
    return {};
}


