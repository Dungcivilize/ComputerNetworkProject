#include "dependencies/framework.hpp"
#include "dependencies/identity.hpp"
#include "dependencies/utils.hpp"

typedef struct
{
    string id;
    string name;
    struct sockaddr_in addr;
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
    if (argc != 2)
    {
        printf("Usage: ./app [PORT]");
        return 0;
    }

    int inp_port = atoi(argv[1]);
    if (inp_port <= 0 || inp_port > 65535)
    {
        printf("Invalid port. Port must be between 0 and 65535");
        return 0;
    }
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