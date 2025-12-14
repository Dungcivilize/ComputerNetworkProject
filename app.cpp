#include "dependencies/framework.hpp"
#include "dependencies/identity.hpp"
#include "dependencies/utils.hpp"
#include "module/index.cpp"

typedef struct
{
    string id;
    string name;
    struct sockaddr_in addr;
    string token;
} DeviceInfo;

typedef struct Device
{
    DeviceInfo info;
    int sockfd;

    Device(DeviceInfo info, int sockfd) : info(info), sockfd(sockfd) {}

    ~Device() { close(sockfd); }
} Device;

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
    for (ssize_t idx = 0; idx < list.size(); idx++)
        if (id == list[idx].id)
            return idx;
    return -1;
}

ssize_t find_device_by_id(vector<Device*> &devices, string id)
{
    for (ssize_t idx = 0; idx < devices.size(); idx++)
        if (id == devices[idx]->info.id)
            return idx;
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

    int connected_devive_id = -1;
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
            cout << "Type LIST to see available devices." << endl;
            cout << "Type HELP to see available commands." << endl;
            cout << "TYPE HELP <command> to see command usage." << endl;
            cout << "===============================" << endl;
            cout << ">> ";

            string input;
            getline(cin, input);
            stringstream ss(input);
            string cmd;
            ss >> cmd;
            if (cmd == "SCAN")
            {
                float duration;
                ss >> duration;
                device_list = broadcast(port, duration);
            }
            else if (cmd == "LIST")
            {
                for (size_t idx = 0; idx < device_list.size(); idx++)
                {
                    DeviceInfo& di = device_list[idx];
                    cout << idx << ": " << di.id << " " << di.name << " "
                         << inet_ntoa(di.addr.sin_addr) << ":"
                         << ntohs(di.addr.sin_port) << endl;
                }
            }
            else if (cmd == "CONNECT")
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
            else if (cmd == "POWER_ON" || cmd == "POWER_OFF")
            {
                if (connected_devive_id < 0)
                {
                    cout << to_string(ERROR_NO_DEVICE_CONNECTED) << " No device connected" << endl;
                    continue;
                }
                call_api(devices[connected_devive_id]->sockfd, cmd + " " + devices[connected_devive_id]->info.token);
            }
            else if (cmd == "WATER_NOW")
            {
                if (connected_devive_id < 0)
                {
                    cout << to_string(ERROR_NO_DEVICE_CONNECTED) << " No device connected" << endl;
                    continue;
                }
                cout << "Enter watering amount in liters (input 0 for default, -1 for cancel): ";
                float amount;
                cin >> amount;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (amount < -1)
                {
                    cout << to_string(ERROR_INVALID_PARAMETER) << " Invalid watering amount" << endl;
                    continue;
                }
                if (amount == -1)
                {
                    cout << "Watering cancelled." << endl;
                    continue;
                }
                call_api(devices[connected_devive_id]->sockfd, cmd + " " + devices[connected_devive_id]->info.token + " " + to_string(amount));
            }
            else if (cmd == "FERTILIZE_NOW")
            {
                if (connected_devive_id < 0)
                {
                    cout << to_string(ERROR_NO_DEVICE_CONNECTED) << " No device connected" << endl;
                    continue;
                }
                cout << "Enter Enter the parameters according to the following pattern: V=2.5 (0 for defaut): ";
                cout << "Enter 'cancel' to cancel fertilizing." << endl;
                cout << "Enter 'done' to proceed." << endl;
                cout << "Parameters list:" << endl;
                cout << "V - Fertilizing volume in liters" << endl;
                cout << "CP - Concentration of phosphorus in mg/L" << endl;
                cout << "CK - Concentration of potassium in mg/L" << endl;
                cout << "CN - Concentration of nitrogen in mg/L" << endl;
                string fert_input;
                map<string, float> fert_params;
                while (true)
                {
                    cout << "Fertilizing parameter: ";
                    getline(cin, fert_input);
                    if (fert_input == "cancel")
                    {
                        cout << "Fertilizing cancelled." << endl;
                        break;
                    }
                    if (fert_input == "done")
                    {
                        stringstream param_ss;
                        param_ss << cmd << " " << devices[connected_devive_id]->info.token << " ";
                        for (const auto& p : fert_params)
                            param_ss << p.first << "=" << p.second << " ";
                        call_api(devices[connected_devive_id]->sockfd, param_ss.str());
                        break;
                    }
                    size_t eq_pos = fert_input.find('=');
                    if (eq_pos == string::npos)
                    {
                        cout << to_string(ERROR_INVALID_PARAMETER) << " Invalid parameter format" << endl;
                        continue;
                    }
                    string key = fert_input.substr(0, eq_pos);
                    float value = stof(fert_input.substr(eq_pos + 1));
                    fert_params[key] = value;
                }
            }
            else if (cmd == "LIGHT_NOW")
            {
                if (connected_devive_id < 0)
                {
                    cout << to_string(ERROR_NO_DEVICE_CONNECTED) << " No device connected" << endl;
                    continue;
                }
                // Nhập thời gian chiếu sáng và công suất ánh sáng
                cout << "Enter lighting duration in minutes: (-1 to cancel) ";
                int duration;
                cin >> duration;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (duration == -1)
                {
                    cout << "Lighting cancelled." << endl;
                    continue;
                }
                cout << "Enter lighting power in Watts: (0 for default, -1 to cancel) ";
                int power;
                cin >> power;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (power == -1)
                {
                    cout << "Lighting cancelled." << endl;
                    continue;
                }
                call_api(devices[connected_devive_id]->sockfd, cmd + " " + devices[connected_devive_id]->info.token + " " + to_string(duration) + " " + to_string(power));
            }
            else if (cmd == "TIMER")
            {
                if (connected_devive_id < 0)
                {
                    cout << to_string(ERROR_NO_DEVICE_CONNECTED) << " No device connected" << endl;
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
                string timer_state = (state == 0) ? "POWER_ON" : "POWER_OFF";
                
                cout << "Enter time to perform action (minutes from now, -1 to cancel): ";
                int minutes;
                cin >> minutes;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (minutes == -1)
                {
                    cout << "Timer setting cancelled." << endl;
                    continue;
                }
                call_api(devices[connected_devive_id]->sockfd, cmd + " " + devices[connected_devive_id]->info.token + " " + timer_state + " " + to_string(minutes));
            }
            else if (cmd == "CANCEL_CONTROL")
            {
                if (connected_devive_id < 0)
                {
                    cout << to_string(ERROR_NO_DEVICE_CONNECTED) << " No device connected" << endl;
                    continue;
                }
                call_api(devices[connected_devive_id]->sockfd, cmd + " " + devices[connected_devive_id]->info.token);
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
            else if (cmd == "QUERY")
            {
                if (connected_devive_id < 0)
                {
                    cout << to_string(ERROR_NO_DEVICE_CONNECTED) << " No device connected" << endl;
                    continue;
                }
                cout << "Enter scope (STATUS, SENSORS, USAGE, CONFIG, ALL or let blank for default, -1 to cancel): ";
                string scope;
                getline(cin, scope);
                if (scope == "-1")
                {
                    cout << "Query cancelled." << endl;
                    continue;
                }
                call_api(devices[connected_devive_id]->sockfd, cmd + " " + devices[connected_devive_id]->info.token + (scope.empty() ? "" : " " + scope));
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