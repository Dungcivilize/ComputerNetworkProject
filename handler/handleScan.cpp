#include "../common/index.hpp"
#include "handleScan.hpp"

vector<Device*> broadcast(uint16_t port, float duration)
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
