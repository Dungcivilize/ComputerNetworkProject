#include <iostream>
#include <string>

// Include sensor class header for the test binary
#include "sensor.hpp"

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cout << "Usage: sensor [PORT] [SENSOR_ID] [[SENSOR_TYPE] [SENSOR_NAME] [SENSOR_PASS]]" << std::endl;
        return 1;
    }
    int port = atoi(argv[1]);
    std::string id = argv[2];
    std::string sensor_type;
    std::string name;
    std::string pass;
    if (argc < 6) {
        FILE* file = fopen(("config/sensor_config" + id + ".txt").c_str(), "r");
        if (file == nullptr) {
            if (argc < 4)
            {
                std::cout << "Usage: sensor [PORT] [SENSOR_ID] [SENSOR_TYPE] [[SENSOR_NAME] [SENSOR_PASS]] to create new sensor" << std::endl;
                return 1;
            }
            sensor_type = argv[3];
            name = "Sensor_" + id;
            pass = "123456";
        } else {
            fscanf(file, "%s %s %s", &sensor_type[0], &name[0], &pass[0]);
            fclose(file);
        }
    } else {
        sensor_type = argv[3];
        name = argv[4];
        pass = argv[5];
    }
    // Lưu lại vào file
    FILE* file = fopen(("config/sensor_config" + id + ".txt").c_str(), "w");
    if (file != nullptr) {
        fprintf(file, "%s %s %s", sensor_type.c_str(), name.c_str(), pass.c_str());
        fclose(file);
    }
    Sensor sensor((uint16_t)port, id, sensor_type, name, pass);
    sensor.run();
    return 0;
}