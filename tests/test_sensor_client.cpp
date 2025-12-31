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
#include <vector>
#include <cstdlib>
#include <fcntl.h>
#include "../common/index.hpp"

using namespace std;

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

// Use shared get_local_ipv4() from dependencies/utils.cpp

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
    vector<int> connected_sensors;

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
        int s = connect_to(ip, port, 1);
        if (s < 0) continue;
        // Send scan command '1'
        send_line(s, "1");
        string resp = recv_line(s);
        if (!resp.empty()) {
            cout << "Found sensor at " << ip << ": " << resp << endl;
        }
        connected_sensors.push_back(s);
        cout << "----------------------------" << endl;
    }

    // Attempt CONNECT to first reachable sensor
    cout << "\nAttempting CONNECT to first reachable sensor..." << endl;
    int s = connected_sensors.empty() ? -1 : connected_sensors[0];
    if (s < 0) {
        cout << "No reachable sensors found." << endl;
        return 1;
    }
    call_api(s, "2 1 password_wrong");
    
    call_api(s, "2 1 password_1");

    call_api(s, "2 1 password_1");    
    // Đóng kết nối
    for (int cs : connected_sensors) {
        close(cs);
    }

    return 0;
}
