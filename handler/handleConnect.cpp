#include "../common/index.hpp"
#include "handleConnect.hpp"

void handleConnect(std::vector<Device*>& devices, int app_id) {
    Device* selected_device = listDeviceToSelect(devices, true);
    if (!selected_device)
        return;
    if (!selected_device->info.token.empty()) {
        cout << " Device already connected. Disconnect? (y/n): ";
        char ch;
        cin >> ch;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (ch == 'y' || ch == 'Y') {
            selected_device->info.token.clear();
            cout << " Device disconnected." << endl;
        return;
    }
    cout << "Enter password to connect: ";
    string password;
    cin >> password;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string token;
    call_api(selected_device->sockfd, to_string(2) + " " + to_string(app_id) + " " + password, (void*)&token);
    if (!token.empty()) {
        selected_device->info.token = token;
    }
}