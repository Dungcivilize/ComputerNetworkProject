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

int main(int argc, char* argv[])
{
    if (argc < 6) {
        cout << "Usage: test_sensor_client [BASE_IP] [START] [END] [PORT] [MY_IP_LAST_OCTET]" << endl;
        cout << "Example: test_sensor_client 192.168.1 2 254 9000 1" << endl;
        return 1;
    }
    string base = argv[1];
    int start = atoi(argv[2]);
    int end = atoi(argv[3]);
    uint16_t port = (uint16_t)atoi(argv[4]);
    int mylast = atoi(argv[5]);

    for (int i = start; i <= end; ++i) {
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

    // Example: try connect to specific address (first successful)
    cout << "\nAttempting CONNECT to first reachable sensor..." << endl;
    for (int i = start; i <= end; ++i) {
        if (i == mylast) continue;
        string ip = base + "." + to_string(i);
        int s = connect_to(ip, port);
        if (s < 0) continue;
        // send connect: 2 <app_id> <password>
        send_line(s, "2 1 password");
        string resp = recv_line(s);
        cout << ip << " -> " << resp << endl;
        close(s);
        break;
    }

    return 0;
}
