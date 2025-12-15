#include "../dependencies/index.hpp"

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
