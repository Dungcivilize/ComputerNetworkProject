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
    std::vector<DeviceInfo> device_list;
    
    cout << "===============================" << endl;
    cout << "Available commands:" << endl;
    cout << " 1. SCAN <duration>               - Scan for devices for <duration> seconds" << endl;
    cout << " 2. CONNECT <id> <password>       - Connect to device with <id> using <password>" << endl;
    cout << " 3. POWER <ON/OFF>               - Power ON/OFF device" << endl;
    cout << " 4. TAKE_CONTROL                - Take control of the device" << endl;
    cout << " 5. TIMER                    - Set timer to power ON/OFF device after specified minutes" << endl;
    cout << " 6. CANCEL_CONTROL                - Cancel current control operations" << endl;
    cout << " 7. CHANGE_PW <id> <current_pw> <new_pw> - Change password for device with <id>" << endl;
    cout << " 8. QUERY                         - Query device for status, sensor data, usage, config or all" << endl;
    cout << " 9. CONFIG <id> <param> <value>  - Change configuration parameter for device with <id>" << endl;
    cout << " 10. EXIT                          - Exit the application" << endl;
    cout << " 11. HELP                          - Show help for commands" << endl;
    cout << "===============================" << endl;
    cout << ">> ";
    string cmd;
    cin >> cmd;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    if (cmd == "1")
    {
        float duration;
        cin >> duration;
        device_list = broadcast(port, duration);
    }
    else if (cmd == "2")
    {
        string id;
        string pass;
        cin >> id >> pass;
        ssize_t idx = find_device_info_by_id(device_list, id);
        if (idx < 0)
            cout << to_string(ERROR_UNKNOWN_TOKEN) << " No such id exist" << endl;
        else
        {
            Device* new_device = establish_connection(device_list[idx], pass);
            if (new_device) devices.push_back(new_device);
        }                   
    }
    else if (cmd == "3")
        power_control(devices);
    else if (cmd == "4")
        take_control(devices);
    else if (cmd == "5")
        set_timer(devices);
    else if (cmd == "6")
        cancel_control(devices);
    else if (cmd == "CHANGE_PW")
    {
        string id, current_pass, new_pass;
        cin >> id >> current_pass >> new_pass;
        ssize_t idx = find_device_by_id(devices, id);
        if (idx < 0)
            cout << to_string(ERROR_UNKNOWN_TOKEN) << " No such id exist" << endl;
        else
            change_password(devices[idx], current_pass, new_pass);
    }
    else if (cmd == "8")
    {
        selected_device = listDeviceToSelect(devices);
        if (!selected_device)
            return;
        call_api(selected_device->sockfd, to_string(8) + " " + selected_device->info.token);
    }
    else if (cmd == "CONFIG")
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
    else if (cmd == "EXIT") return;
    else
        cout << to_string(ERROR_UNKNOWN_COMMAND) << " Uknown command" << endl;
}
