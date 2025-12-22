#include <iostream>
#include <string>

// Include sensor implementation directly for the test binary
#include "../sensor/sensor.cpp"

int main(int argc, char* argv[])
{
    if (argc < 5) {
        std::cout << "Usage: test_sensor [PORT] [SENSOR_ID] [NAME] [PASSWORD]" << std::endl;
        return 1;
    }
    int port = atoi(argv[1]);
    std::string id = argv[2];
    std::string name = argv[3];
    std::string pass = argv[4];

    Sensor sensor((uint16_t)port, id, name, pass);
    sensor.run();
    return 0;
}
