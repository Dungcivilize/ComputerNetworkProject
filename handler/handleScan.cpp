#include "../common/index.hpp"
#include "handleScan.hpp"

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

vector<Device*> scan_devices(uint16_t port)
{
    vector<Device*> found_devices;
    string local_ip;
    if (!get_local_ipv4(local_ip)) {
        cerr << "Failed to detect local IPv4 address." << endl;
        throw runtime_error("No local IPv4");
    }
    // extract base (first 3 octets) and last octet
    vector<string> parts;

    stringstream sp(local_ip);
    string item;
    while (getline(sp, item, '.')) parts.push_back(item);
    if (parts.size() != 4) {
        cerr << "Unexpected local IP format: " << local_ip << endl;
        throw runtime_error("Unexpected local IP format");
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
        string response;
        recv_line(s, response);
        if (!response.empty()) {
            cout << "Found sensor at " << ip << ": " << response << endl;
        }
        found_devices.push_back(new Device(ip, port, s, response));
        cout << "----------------------------" << endl;
    }
    cout << "Scan complete. Found " << found_devices.size() << " device(s)." << endl;
    return found_devices;
}