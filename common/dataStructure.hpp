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
    std::string token;
    enum DeviceType type;
} DeviceInfo;

typedef struct Device
{
    DeviceInfo info;
    int sockfd;

    Device(int ip, string info_str)
    {
        sockfd = ip;
        stringstream ss(info_str);
        string return_code;
        ss >> return_code;
        if (return_code != "100")
        {
            throw std::runtime_error("Invalid device info string");
        }
        string type;
        ss >> info.id >> info.name >> type;
        if (type == "SPRINKLER")
            info.type = SPRINKLER;
        else if (type == "FERTILIZER")
            info.type = FERTILIZER;
        else if (type == "LIGHTING")
            info.type = LIGHTING;
    }

    ~Device() { close(sockfd); }
} Device;

#endif // DATASTRUCTURE_HPP
