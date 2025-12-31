#include "streamtransmission.hpp"

ssize_t send_all(int fd, char* buf, size_t len)
{
    size_t total_sent = 0;
    while (total_sent < len)
    {
        ssize_t n = send(fd, buf + total_sent, len - total_sent, 0);
        if (n < 0)
        {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) break;
        total_sent += (size_t)n;
    }
    return (ssize_t)total_sent;
}

ssize_t recv_line(int fd, char* buf, size_t len)
{
    size_t count = 0;
    while (count < len - 1)
    {
        char c = 0;
        ssize_t n = recv(fd, &c, 1, 0);
        if (n == 1)
        {
            buf[count++] = c;
            if (c == '\n') break;
        }   
        else if (n == 0)
            break;
        else
        {
            if (errno == EINTR) continue;
            return -1;
        }
    }
    buf[count] = '\0';
    return (ssize_t)count;
}