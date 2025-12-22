#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <errno.h>
#include <iostream>
#include <string>
#include <sstream>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>

using namespace std;

int connect_to(const string &ip, uint16_t port, int timeout_sec = 2)
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
    // set timeout
    struct timeval tv;
    tv.tv_sec = timeout_sec;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}

bool send_line(int sock, const string &msg)
{
    string s = msg + "\n";
    ssize_t n = send(sock, s.c_str(), s.size(), 0);
    return n == (ssize_t)s.size();
}

string recv_line(int sock)
{
    string res;
    char c;
    while (true) {
        ssize_t n = recv(sock, &c, 1, 0);
        if (n <= 0) break;
        if (c == '\n') break;
        res.push_back(c);
    }
    return res;
}

static bool get_local_ipv4(string &out_ip)
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

int main(int argc, char* argv[])
{
    uint16_t port = 9000;
    if (argc >= 2) port = (uint16_t)atoi(argv[1]);

    string local_ip;
    if (!get_local_ipv4(local_ip)) {
        cerr << "Failed to detect local IPv4 address." << endl;
        return 1;
    }
    // extract base (first 3 octets) and last octet
    vector<string> parts;
    stringstream sp(local_ip);
    string item;
    while (getline(sp, item, '.')) parts.push_back(item);
    if (parts.size() != 4) {
        cerr << "Unexpected local IP format: " << local_ip << endl;
        return 1;
    }
    string base = parts[0] + "." + parts[1] + "." + parts[2];
    int mylast = atoi(parts[3].c_str());

    cout << "Local IP detected: " << local_ip << " — scanning " << base << ".1-254 on port " << port << " (skipping " << mylast << ")" << endl;

    for (int i = 1; i <= 254; ++i) {
        if (i == mylast) continue;
        string ip = base + "." + to_string(i);
        int s = connect_to(ip, port);
        if (s < 0) continue;
        // Send scan command '1'
        send_line(s, "1");
        string resp = recv_line(s);
        if (!resp.empty()) {
            cout << "Found sensor at " << ip << ": " << resp << endl;
        }
        close(s);
    }

    // Attempt CONNECT to first reachable sensor
    cout << "\nAttempting CONNECT to first reachable sensor..." << endl;
    for (int i = 1; i <= 254; ++i) {
        if (i == mylast) continue;
        string ip = base + "." + to_string(i);
        int s = connect_to(ip, port);
        if (s < 0) continue;
        send_line(s, "2 1 password");
        string resp = recv_line(s);
        cout << ip << " -> " << resp << endl;
        close(s);
        break;
    }

    return 0;
}
