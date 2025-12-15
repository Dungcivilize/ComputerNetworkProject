#ifndef DATASTRUCTURE_HPP
#define DATASTRUCTURE_HPP

#include "../dependencies/index.hpp"

typedef struct
{
    std::string id;
    std::string name;
    struct sockaddr_in addr;
    std::string token;
} DeviceInfo;

typedef struct Device
{
    DeviceInfo info;
    int sockfd;

    Device(DeviceInfo info, int sockfd) : info(info), sockfd(sockfd) {}

    ~Device() { close(sockfd); }
} Device;

#endif // DATASTRUCTURE_HPP
