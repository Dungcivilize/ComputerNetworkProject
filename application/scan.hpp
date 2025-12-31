#pragma once

#define CONNECTION_TIME_LIMIT 5
#define RESPONSE_TIME_LIMIT 100

#include "framework.hpp"
#include "device.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

/* Get current local IPv4 address assuming there's only one network. */
static bool get_localaddr(char* network, char* broadcast, int& prefix)
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
        prefix = __builtin_popcount(mask);

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
    communicate(fd, "SCAN", buf, resp, exec_code);
    string message = string("SCAN ") + (exec_code == SUCCESS_SCAN ? "OK:" : "ERR:") + to_string(exec_code) + " " + resp;
    logging(PLOG, message);
    return exec_code == SUCCESS_SCAN;
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

    rc = poll(&pfd, 1, CONNECTION_TIME_LIMIT);
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
    tv.tv_usec = RESPONSE_TIME_LIMIT * 1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
    bool rs = ask(fd, id, resp, exec_code);
    close(fd);
    return rs;
}

/* Parse message if success. */
static bool parse_scan_reply(const string& resp, string& id, string& type)
{   
    vector<string> tokens = parse_info_message(resp);
    if (tokens.empty()) return false;
    id = tokens[0];
    type = tokens[1];
    return true;
}

/* Scan all host addresses in the local network for devices. */
static bool scan(vector<DeviceInfo>& devices, const string& id)
{
    if (!devices.empty()) devices.clear();

    // Buffers
    string resp;
    int exec_code = 0;

    int so[4], eo[4], prefix;
    char net[INET_ADDRSTRLEN], bcast[INET_ADDRSTRLEN], ip[INET_ADDRSTRLEN];
    if (!get_localaddr(net, bcast, prefix))
    {
        cerr << "Cannot retrieve current local IPv4 address" << endl;
        return false;
    }
    sscanf(net, "%d.%d.%d.%d", &so[0], &so[1], &so[2], &so[3]);
    sscanf(bcast, "%d.%d.%d.%d", &eo[0], &eo[1], &eo[2], &eo[3]);
    so[3]++;
    double expected_wait_time = (pow(2, 32 - prefix) - 2) * CONNECTION_TIME_LIMIT / 1000;
    cout << "Begin scanning. Expected wait time: " + to_string(expected_wait_time) + "s" << endl;
    
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
            string device_id, type;
            if (!parse_scan_reply(resp, device_id, type))
                continue;
            devices.push_back(DeviceInfo(sa, device_id, type));
            cout << "Found " + type + " " + device_id + " at " + string(ip) << endl;
        }
    }
    cout << "Scan finished. Found " + to_string(devices.size()) + " devices" << endl;
    return true;
}