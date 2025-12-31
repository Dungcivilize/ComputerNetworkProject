#pragma once

#include "framework.hpp"

static ssize_t send_all(int fd, const void* buf, size_t len)
{
    const char* p = (const char*)buf;
    size_t total_sent = 0;

    while (total_sent < len) 
    {
        ssize_t n = send(fd, p + total_sent, len - total_sent, 0);
        if (n < 0) 
        {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0)
        {
            errno = ECONNRESET;
            return -1;
        }
        total_sent += (size_t)n;
    }
    return (ssize_t)total_sent;
}

static ssize_t recv_all(int fd, void* buf, size_t len)
{
    char* p = (char*)buf;
    size_t total = 0;

    while (total < len) 
    {
        ssize_t n = recv(fd, p + total, len - total, 0);
        if (n < 0) 
        {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) 
        {
            errno = ECONNRESET;
            return -1;
        }
        total += (size_t)n;
    }
    return (ssize_t)total;
}

/* TCP message sending function for C++ string. */
static bool send_message(int fd, const string& message)
{
    uint32_t len = (uint32_t)message.size();
    uint32_t net = htonl(len);

    if (send_all(fd, &net, sizeof(net)) != (ssize_t)sizeof(net))
        return false;

    if (len == 0) return true;

    return send_all(fd, message.data(), len) == (ssize_t)len;
}

/* TCP message receive function for C++ string. */
static bool recv_message(int fd, string& message)
{
    uint32_t net = 0;
    if (recv_all(fd, &net, sizeof(net)) != (ssize_t)sizeof(net))
        return false;

    uint32_t len = ntohl(net);
    message.resize(len);

    if (len == 0) return true;

    return recv_all(fd, message.data(), len) == (ssize_t)len;
}