#include "framework.hpp"
#include "streamtransmission.hpp"

void trim(char* s) 
{
    size_t n = strlen(s);
    while (n && (s[n-1] == '\n' || s[n-1] == '\r')) s[--n] = '\0';
}

ssize_t send_message(int sockfd, string &message)
{
    char buf[BUFFER_SIZE];
    int n = snprintf(buf, sizeof(buf), "%s\n", message.c_str());
    if (n < 0)
        return -1;
    if (n >= (int)sizeof(buf))
    {   
        n = (int)sizeof(buf);
        buf[n - 1] = '\n';
    }
    return send_all(sockfd, buf, n);
}

ssize_t recv_message(int sockfd, string& buf)
{
    char buffer[BUFFER_SIZE];
    ssize_t n = recv_line(sockfd, buffer, sizeof(buffer));
    if (n <= 0) return n;
    trim(buffer);
    buf.assign(buffer);
    return n;
}