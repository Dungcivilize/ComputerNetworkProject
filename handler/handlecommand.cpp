#include "handlecommand.hpp"

#include "handleScan.hpp"
#include "handleConnect.hpp"
#include "handleControl.hpp"
#include "handleChangeInfo.hpp"

#include "../common/index.hpp"
#include "utils.hpp"
#include <iostream>
#include <sstream>

// Hàm xử lý lệnh nhập vào, tách từng lệnh và gọi module tương ứng
void handleCommand(std::vector<Device*>& devices, uint16_t port) {
    Device* selected_device = nullptr;
    system("clear");
    cout << "===============================" << endl;
    cout << "Available commands:" << endl;
    cout << " 1. SCAN                        - Scan for devices" << endl;
    cout << " 2. CONNECT <id> <password>       - Connect to device with <id> using <password>" << endl;
    cout << " 3. CONTROL DEVICE                 - Take control of a device" << endl;
    cout << " 4. CHANGE_PW <id> <current_pw> <new_pw> - Change password for device with <id>" << endl;
    cout << " 5. QUERY                         - Query device for status, sensor data, usage, config or all" << endl;
    cout << " 6. CONFIG <id> <param> <value>  - Change configuration parameter for device with <id>" << endl;
    cout << " 7. EXIT                          - Exit the application" << endl;
    cout << " 8. HELP                          - Show help for commands" << endl;
    cout << "===============================" << endl;
    cout << ">> ";
    string cmd;
    cin >> cmd;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    if (cmd == "1")
    {
        devices = scan_devices(port);
    }
    else if (cmd == "2")
    {

    }
    else if (cmd == "3")
    {
        handleControl(devices);
    }
    else if (cmd == "4")
    {
        string id, current_pass, new_pass;
        cin >> id >> current_pass >> new_pass;
        ssize_t idx = find_device_by_id(devices, id);
        if (idx < 0)
            cout << to_string(ERROR_UNKNOWN_TOKEN) << " No such id exist" << endl;
        else
            change_password(devices[idx], current_pass, new_pass);
    }
    else if (cmd == "5")
    {
        selected_device = listDeviceToSelect(devices);
        if (!selected_device)
            return;
        call_api(selected_device->sockfd, to_string(8) + " " + selected_device->info.token);
    }
    else if (cmd == "6")
    {
        string id, param;
        float value;
        cin >> id >> param >> value;
        ssize_t idx = find_device_by_id(devices, id);
        if (idx < 0)
            cout << to_string(ERROR_UNKNOWN_TOKEN) << " No such id exist" << endl;
        else
            change_param(devices[idx], param, value);
    }
    else if (cmd == "7") return;
    else
        cout << to_string(ERROR_UNKNOWN_COMMAND) << " Uknown command" << endl;
}
