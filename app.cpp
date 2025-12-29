#include "dependencies/index.hpp"
#include "common/index.hpp"
#include "handler/handlecommand.hpp"

int main(int argc, char** argv)
{
    int inp_port = check_args(argc, argv);
    if (inp_port < 0)
        return 1;

    int app_id = register_app();
    printf("App registered with ID: %d\n", app_id);

    uint16_t port = (uint16_t)inp_port;
    vector<DeviceInfo> device_list;
    vector<Device*> devices;
    while (handleCommand(devices, port, app_id)) {
        // Loop until user decides to exit
    }

    for (auto* dev : devices)
        delete dev;
    return 0;
}