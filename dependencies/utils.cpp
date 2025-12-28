#include "utils.hpp"
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

void trim(char* s) 
{
    size_t n = strlen(s);
    while (n && (s[n-1] == '\n' || s[n-1] == '\r')) s[--n] = '\0';
}

ssize_t send_message(int sockfd, std::string &message)
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

ssize_t recv_message(int sockfd, std::string& buf)
{
    char buffer[BUFFER_SIZE];
    ssize_t n = recv_line(sockfd, buffer, sizeof(buffer));
    if (n <= 0) return n;
    trim(buffer);
    buf.assign(buffer);
    return n;
}


bool get_local_ipv4(string &out_ip)
{
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) return false;
    bool found = false;
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family == AF_INET) {
            // skip loopback
            if (ifa->ifa_flags & IFF_LOOPBACK) continue;
            char host[NI_MAXHOST];
            int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s == 0) {
                out_ip = string(host);
                found = true;
                break;
            }
        }
    }
    freeifaddrs(ifaddr);
    return found;
}

int connect_to(const string &ip, uint16_t port, int timeout_ms = 200)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        close(sock);
        return -1;
    }
    // make socket non-blocking for timed connect
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) flags = 0;
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(sock);
        return -1;
    }

    int res = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (res == 0) {
        // connected immediately
        // restore blocking
        fcntl(sock, F_SETFL, flags);
    } else if (res < 0) {
        if (errno != EINPROGRESS) {
            close(sock);
            return -1;
        }

        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(sock, &wfds);
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        int sel = select(sock + 1, NULL, &wfds, NULL, &tv);
        if (sel <= 0) {
            // timeout or error
            close(sock);
            return -1;
        }

        int so_error = 0;
        socklen_t len = sizeof(so_error);
        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
            close(sock);
            return -1;
        }
        if (so_error != 0) {
            close(sock);
            return -1;
        }

        // restore blocking mode
        fcntl(sock, F_SETFL, flags);
    }

    // set small recv/send timeouts so later IO won't hang long
    struct timeval tv2;
    tv2.tv_sec = 0;
    tv2.tv_usec = 300 * 1000; // 300 ms
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv2, sizeof tv2);
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv2, sizeof tv2);
    return sock;
}
