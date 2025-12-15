#include "dependencies/index.hpp"
#include "module/index.hpp"

vector<DeviceInfo> broadcast(uint16_t port, float duration)
{
    vector<DeviceInfo> devices;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    int opt = 1;
    if (setsockopt(sockfd,SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0)
    {
        perror("broadcast.setsockopt");
        return devices;
    }
    
    struct sockaddr_in baddr;
    baddr.sin_family = AF_INET;
    baddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    baddr.sin_port = htons(port);

    ssize_t sent = sendto(sockfd, BROADCAST_MESSAGE, strlen(BROADCAST_MESSAGE), 0, (struct sockaddr*)&baddr, sizeof(baddr));
    if (sent < 0)
    {
        perror("broadcast.sendto");
        return devices;
    }

    struct timeval tv;
    tv.tv_sec = duration;
    tv.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        perror("broadcast.setsockopt");

    while (1)
    {
        char buf[BUFFER_SIZE];
        struct sockaddr_in source;
        socklen_t source_len = sizeof(source);

        ssize_t n = recvfrom(sockfd, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&source, &source_len);
        if (n < 0)
        {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            perror("broadcast.recvfrom");
            break;
        }
        if (n == 0) break;
        buf[n] = '\0';
        
        char type[TYPE_SIZE], id[ID_SIZE], name[NAME_SIZE];
        int scanned = sscanf(buf, "%16s %127s %127s", type, id, name);
        if (scanned == 3)
        {
            DeviceInfo di;
            di.id = id;
            di.name = name;
            di.addr = source;
            printf("%s %s %s\n", type, id, name);
            devices.push_back(di);
        }
    }
    return devices;
}


ssize_t find_device_info_by_id(vector<DeviceInfo> &list, string id)
{
    for (size_t idx = 0; idx < list.size(); idx++)
        if (id == list[idx].id)
            return (ssize_t)idx;
    return -1;
}

ssize_t find_device_by_id(vector<Device*> &devices, string id)
{
    for (size_t idx = 0; idx < devices.size(); idx++)
        if (id == devices[idx]->info.id)
            return (ssize_t)idx;
    return -1;
}

Device* establish_connection(DeviceInfo info, string password)
{
    int devfd = socket(AF_INET, SOCK_STREAM, 0);
    if (devfd < 0)
    {
        perror("establish_connection.socket");
        return NULL;
    }
    struct sockaddr_in devaddr;
    devaddr.sin_family = AF_INET;
    devaddr.sin_addr = info.addr.sin_addr;
    devaddr.sin_port = info.addr.sin_port;
    if (connect(devfd, (struct sockaddr*)&devaddr, sizeof(devaddr)) < 0)
    {
        perror("establish_connection.connect");
        return NULL;
    }
    string buf = "CONNECT " + password;
    send_message(devfd, buf);
    recv_message(devfd, buf);
    cout << buf << endl;
    stringstream ss(buf);
    int code = 0;
    ss >> code;
    if (code == SUCCESS_CONNECTION)
        return new Device(info, devfd);
    else
        close(devfd);
    return NULL;
}

void change_password(Device* device, string current_password, string new_password)
{
    string buf = "CHANGE_PW " + current_password + " " + new_password;
    send_message(device->sockfd, buf);
    recv_message(device->sockfd, buf);
    cout << buf << endl;
}

void change_param(Device* device, string param, float value)
{
    string buf = "CONFIG " + param + " " + to_string(value);
    send_message(device->sockfd, buf);
    recv_message(device->sockfd, buf);
    cout << buf << endl;
}

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
    while (1)
    {
        fd_set rfds;
        FD_ZERO(&rfds);

        FD_SET(STDIN_FILENO, &rfds);
        int maxfd = STDIN_FILENO;

        for (auto* device : devices)
        {
            FD_SET(device->sockfd, &rfds);
            if (device->sockfd > maxfd)
                maxfd = device->sockfd;
        }

        int ready = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (ready < 0)
        {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &rfds))
        {
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
            int cmd;
            cin >> cmd;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (cmd == 1)
            {
                float duration;
                ss >> duration;
                device_list = broadcast(port, duration);
            }
            else if (cmd == 2)
            {
                string id;
                string pass;
                ss >> id >> pass;
                ssize_t idx = find_device_info_by_id(device_list, id);
                if (idx < 0)
                    cout << to_string(ERROR_UNKNOWN_TOKEN) << " No such id exist" << endl;
                else
                {
                    Device* new_device = establish_connection(device_list[idx], pass);
                    if (new_device) devices.push_back(new_device);
                }                   
            }
            else if (cmd == 3)
            {
                Device* selected_device = listDeviceToSelect(devices);
                if (!selected_device)
                    continue;
                if (selected_device->info.token.empty()) {
                    cout << " Device not connected properly." << endl;
                    continue;
                }
                cout << "Enter power state (0 for ON, 1 for OFF): ";
                int state;
                cin >> state;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (state != 0 && state != 1) {
                    cout << " Invalid power state." << endl;
                    continue;
                }
                string power_state = (state == 0) ? "POWER_ON" : "POWER_OFF";
                call_api(selected_device->sockfd, to_string(cmd) + " " + selected_device->info.token + " " + power_state);
            }
            else if (cmd == 4)
            {
                Device* selected_device = listDeviceToSelect(devices);
                if (!selected_device)
                    continue;
                if (selected_device->info.token.empty()) {
                    cout << " Device not connected properly." << endl;
                    continue;
                }
                if (selected_device->info.type == SPRINKLER) {
                    cout << "Enter watering amount in liters (input 0 for default, -1 to cancel): ";
                    int amount;
                    cin >> amount;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (amount == -1) {
                        cout << "Watering cancelled." << endl;
                        continue;
                    }
                    call_api(selected_device->sockfd, to_string(cmd) + " " + selected_device->info.token + " " + to_string(amount));
                }
                else if (selected_device->info.type == FERTILIZER) {
                    // Nhập nồng độ phân đạm
                    cout << "Enter the Concentration of nitrogen in mg/L (input 0 for default, -1 to cancel): ";
                    int cn;
                    cin >> cn;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (cn == -1) {
                        cout << "Fertilizing cancelled." << endl;
                        continue;
                    }
                    cout << "Enter the Concentration of phosphorus in mg/L (input 0 for default, -1 to cancel): ";
                    int cp;
                    cin >> cp;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (cp == -1) {
                        cout << "Fertilizing cancelled." << endl;
                        continue;
                    }
                    cout << "Enter the Concentration of potassium in mg/L (input 0 for default, -1 to cancel): ";
                    int ck;
                    cin >> ck;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (ck == -1) {
                        cout << "Fertilizing cancelled." << endl;
                        continue;
                    }
                    cout << "Enter the volume to fertilize in liters (input 0 for default, -1 to cancel): ";
                    int volume;
                    cin >> volume;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (volume == -1) {
                        cout << "Fertilizing cancelled." << endl;
                        continue;
                    }
                    call_api(selected_device->sockfd, to_string(cmd) + " " + selected_device->info.token + " " + to_string(cn) + " " + to_string(cp) + " " + to_string(ck) + " " + to_string(volume));
                }
                else if (selected_device->info.type == LIGHTING) {
                    cout << "Enter lighting duration in minutes (input -1 to cancel): ";
                    int duration;
                    cin >> duration;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (duration == -1) {
                        cout << "Lighting cancelled." << endl;
                        continue;
                    }
                    cout << "Enter lighting power in Watts (input 0 for default, -1 to cancel): ";
                    int power;
                    cin >> power;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (power == -1) {
                        cout << "Lighting cancelled." << endl;
                        continue;
                    }
                    call_api(selected_device->sockfd, to_string(cmd) + " " + selected_device->info.token + " " + to_string(duration) + " " + to_string(power));
                }
            }
            else if (cmd == 5)
            {
                Device* selected_device = listDeviceToSelect(devices);
                if (!selected_device)
                    continue;
                if (selected_device->info.token.empty()) {
                    cout << " Device not connected properly." << endl;
                    continue;
                }
                cout << "Enter state (0 to ON, 1 to OFF, -1 to cancel): ";
                int state;
                cin >> state;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (state == -1)
                {
                    cout << "Timer setting cancelled." << endl;
                    continue;
                }
                
                cout << "Enter time to perform action (minutes from now, -1 to cancel): ";
                int minutes;
                cin >> minutes;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (minutes == -1)
                {
                    cout << "Timer setting cancelled." << endl;
                    continue;
                }
                call_api(selected_device->sockfd, to_string(cmd) + " " + selected_device->info.token + " " + to_string(state) + " " + to_string(minutes));
            }
            else if (cmd == 6)
            {
                Device* selected_device = listDeviceToSelect(devices);
                if (!selected_device)
                    continue;
                if (selected_device->info.token.empty()) {
                    cout << " Device not connected properly." << endl;
                    continue;
                }
                call_api(selected_device->sockfd, to_string(cmd) + " " + selected_device->info.token);
            }
            else if (cmd == "CHANGE_PW")
            {
                string id, current_pass, new_pass;
                ss >> id >> current_pass >> new_pass;
                ssize_t idx = find_device_by_id(devices, id);
                if (idx < 0)
                    cout << to_string(ERROR_UNKNOWN_TOKEN) << " No such id exist" << endl;
                else
                    change_password(devices[idx], current_pass, new_pass);
            }
            else if (cmd == 8)
            {
                Device* selected_device = listDeviceToSelect(devices);
                if (!selected_device)
                    continue;
                if (selected_device->info.token.empty()) {
                    cout << " Device not connected properly." << endl;
                    continue;
                }
                call_api(selected_device->sockfd, to_string(cmd) + " " + selected_device->info.token);
            }
            else if (cmd == "CONFIG")
            {
                string id, param;
                float value;
                ss >> id >> param >> value;
                ssize_t idx = find_device_by_id(devices, id);
                if (idx < 0)
                    cout << to_string(ERROR_UNKNOWN_TOKEN) << " No such id exist" << endl;
                else
                    change_param(devices[idx], param, value);
            }
            else if (cmd == "EXIT") break;
            else
                cout << to_string(ERROR_UNKNOWN_COMMAND) << " Uknown command" << endl;
        }

        for (size_t idx = 0; idx < devices.size();)
        {
            Device* dev = devices[idx];

            if (FD_ISSET(dev->sockfd, &rfds))
            {
                string buf;
                ssize_t n = recv_message(dev->sockfd, buf);
                if (n <= 0)
                {
                    if (n < 0)
                        perror("recv_message");
                    cout << "Device " << dev->info.id << " disconnected.";

                    delete dev;
                    devices.erase(devices.begin() + idx);
                    continue;
                }
                else
                {
                    cout << buf << endl;
                }
            }
            idx++;
        }
    }

    for (auto* dev : devices)
        delete dev;
    return 0;
}