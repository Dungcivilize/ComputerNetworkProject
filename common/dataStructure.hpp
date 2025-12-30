#ifndef DATASTRUCTURE_HPP
#define DATASTRUCTURE_HPP

#include "../dependencies/index.hpp"

enum DeviceType
{
    SPRINKLER,
    FERTILIZER,
    LIGHTING
};

typedef struct
{
    std::string id;
    std::string name;
    std::string addr_str;
    std::string token;
    enum DeviceType type;
} DeviceInfo;

typedef struct Device
{
    DeviceInfo info;
    int sockfd;

    Device(std::string ip, int port, int sockfd, const std::string& info_str)
    {
        cout << "Creating device from info string: " << info_str << endl;
        this->sockfd = sockfd;
        stringstream ss(info_str);
        string return_code;
        ss >> return_code;
        if (return_code != "100")
        {
            throw std::runtime_error("Invalid device info string");
        }
        string type;
        ss >> info.id >> info.name >> type;
        cout << "Parsed device info - ID: " << info.id << ", Name: " << info.name << ", Type: " << type << endl;
        if (type == "SPRINKLER")
            info.type = SPRINKLER;
        else if (type == "FERTILIZER")
            info.type = FERTILIZER;
        else if (type == "LIGHTING")
            info.type = LIGHTING;
        // Tạo địa chỉ thiết bị
        info.addr_str = ip + ":" + std::to_string(port);
    }

    ~Device() { close(sockfd); }
} Device;

#endif // DATASTRUCTURE_HPP
