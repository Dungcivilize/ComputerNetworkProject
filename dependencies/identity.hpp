#include "framework.hpp"

struct Identity
{
    int sockfd;
    struct sockaddr_in addr;

    Identity(int sockfd, struct sockaddr_in addr) : sockfd(sockfd), addr(addr) {}

    ~Identity() { close(sockfd); }
};

Identity* create_identity(uint16_t port, int type, int level, int optname, in_addr_t inaddr)
{
    int fd = socket(AF_INET, type, 0);
    if (fd < 0)
    {
        perror("create_identity.socket");
        return NULL;
    }
    
    int opt = 1;
    if (setsockopt(fd, level, optname, &opt, sizeof(opt)) < 0) 
    {
        perror("create_identity.setsocketopt");
        close(fd);
        return NULL;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(inaddr);
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("create_identity.bind");
        close(fd);
        return NULL;
    }

    return new Identity(fd, addr);
}