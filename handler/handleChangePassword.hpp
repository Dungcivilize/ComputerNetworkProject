#include "../common/index.hpp"

void handleChangePassword(std::vector<Device*>& devices) {
    Device* selected_device = listDeviceToSelect(devices, true);
    if (!selected_device)
        return;
    cout << "Enter current password: ";
    string current_pass;
    cin >> current_pass;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter new password: ";
    string new_pass;
    cin >> new_pass;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string response;
    call_api(selected_device->sockfd, to_string(4) + " " + selected_device->info.token + " " + current_pass + " " + new_pass);
}