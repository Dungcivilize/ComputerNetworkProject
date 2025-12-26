#pragma once

#include "framework.hpp"
#include "device.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

/* Get current local IPv4 address assuming there's only one network. */
static bool get_localaddr(char* network, char* broadcast)
{
    struct ifaddrs *ifaddr = NULL;
    if (getifaddrs(&ifaddr) == -1) 
    {
        perror("getifaddrs");
        return false;
    }

    bool found = false;
    for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (!ifa->ifa_addr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;
        if (ifa->ifa_flags & IFF_LOOPBACK) continue;

        struct sockaddr_in* sa_ip = (struct sockaddr_in*)ifa->ifa_addr;
        struct sockaddr_in* sa_sm = (struct sockaddr_in*)ifa->ifa_netmask;
        
        uint32_t ip = ntohl(sa_ip->sin_addr.s_addr);
        uint32_t mask = ntohl(sa_sm->sin_addr.s_addr);

        uint32_t net = ip & mask;
        uint32_t bcast = net | (~mask);

        struct in_addr tmp_net;
        struct in_addr tmp_bcast;
        tmp_net.s_addr = htonl(net);
        tmp_bcast.s_addr = htonl(bcast);
        if (!inet_ntop(AF_INET, &tmp_net, network, INET_ADDRSTRLEN) || !inet_ntop(AF_INET, &tmp_bcast, broadcast, INET_ADDRSTRLEN)) 
        {
            perror("inet_ntop");
            continue;
        }
        
        found = true;
        break;
    }
    freeifaddrs(ifaddr);
    return found;
}

/* Try TCP communication. */
static bool ask(int fd, const string& id, string& resp, int& exec_code)
{
    string buf = "SCAN " + id;

    if (!send_message(fd, buf))
    {
        exec_code = ERROR_NO_CONNECTION;
        return false;
    } 
    if (!recv_message(fd, buf))
    {
        exec_code = ERROR_NO_CONNECTION;
        return false;
    } 

    stringstream ss(buf);
    string cmd;
    ss >> cmd;
    if (cmd == "SCAN")
    {
        ss >> exec_code;
        auto p1 = buf.find(' ');
        auto p2 = buf.find(' ', p1 + 1);
        resp = buf.substr(p2 + 1);
    }
    else
    {
        exec_code = ERROR_BAD_REQUEST;
        return false;
    }
    return true;
}

/* Try probing address with time limit. */
static bool try_connect(sockaddr_in sa, const string& id, string& resp, int& exec_code)
{ 
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return false;
    }

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        close(fd);
        return false;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) 
    { 
        close(fd); 
        return false; 
    }

    int rc = connect(fd, (sockaddr*)&sa, sizeof(sa));
    if (!rc)
    {
        fcntl(fd, F_SETFL, flags);
        bool rs = ask(fd, id, resp, exec_code);
        close(fd);
        return rs;
    }
    if (rc < 0 && errno != EINPROGRESS) 
    {
        close(fd);
        return false;
    }

    pollfd pfd{};
    pfd.fd = fd;
    pfd.events = POLLOUT;

    rc = poll(&pfd, 1, CONN_TIME_LIMIT);
    if (rc == 0) 
    {
        close(fd);
        errno = ETIMEDOUT;
        return false;
    }
    if (rc < 0) 
    {
        close(fd);
        return false;
    }

    int so_error = 0;
    socklen_t slen = sizeof(so_error);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &slen) < 0) 
    {
        close(fd);
        return false;
    }
    if (so_error != 0) 
    {
        close(fd);
        errno = so_error;
        return false;
    }

    fcntl(fd, F_SETFL, flags);
    
    timeval tv{};
    tv.tv_sec  = 0;
    tv.tv_usec = RESP_TIME_LIMIT * 1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
    bool rs = ask(fd, id, resp, exec_code);
    close(fd);
    return rs;
}

/* Parse message if success. */
static bool parse_scan_reply(const string& resp, string& id, string& name, string& type)
{   
    vector<string> tokens = parse_info_message(resp);
    if (tokens.empty()) return false;
    id = tokens[0];
    name = tokens[1];
    type = tokens[2];
    return true;
}

/* Scan all host addresses in the local network for devices. */
static bool scan(vector<DeviceInfo>& devices, const string& id)
{
    if (!devices.empty()) devices.clear();

    // Buffers
    string resp;
    int exec_code = 0;

    int so[4], eo[4];
    char net[INET_ADDRSTRLEN], bcast[INET_ADDRSTRLEN], ip[INET_ADDRSTRLEN];
    if (!get_localaddr(net, bcast)) return false;
    sscanf(net, "%d.%d.%d.%d", &so[0], &so[1], &so[2], &so[3]);
    sscanf(bcast, "%d.%d.%d.%d", &eo[0], &eo[1], &eo[2], &eo[3]);
    so[3]++;
    
    // Scan on local network
    while (!(so[0] == eo[0] && so[1] == eo[1] && so[2] == eo[2] && so[3] == eo[3]))
    {
        for (int i = 3; i >= 0; i--)
            if (so[i] > 255 && i > 0)
            {
                so[i] = 0;
                so[i - 1]++;
            }
        
        snprintf(ip, INET_ADDRSTRLEN, "%d.%d.%d.%d", so[0], so[1], so[2], so[3]);
        so[3]++;
        
        sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_port = htons(PORT);
        if (inet_pton(AF_INET, ip, &sa.sin_addr) != 1)
            continue;
        
        if (try_connect(sa, id, resp, exec_code))
        {
            string device_id, name, type;
            if (!parse_scan_reply(resp, device_id, name, type))
                continue;
            devices.push_back(DeviceInfo(sa, device_id, name, type));
        }
    }
 
    return true;
}