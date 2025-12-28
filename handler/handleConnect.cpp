#include "../common/index.hpp"
#include "handleConnect.hpp"

void handleConnect(std::vector<Device*>& devices, int app_id) {
    Device* selected_device = listDeviceToSelect(devices);
    if (!selected_device)
        return;
    cout << "Enter password to connect: ";
    string password;
    cin >> password;
    string token;
    call_api(selected_device->sockfd, to_string(2) + " " + to_string(app_id) + " " + password, (void*)&token);
    if (!token.empty()) {
        selected_device->info.token = token;
    }
}