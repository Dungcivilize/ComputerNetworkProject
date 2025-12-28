#include "../common/index.hpp"
#include "handleScan.hpp"
#include "../dependencies/utils.hpp"

#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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
        if (send_message(s, "1") < 0) {
            close(s);
            cout << "Failed to send scan message to " << ip << endl;
            continue;
        }
        string response;
        
        if (recv_message(s, response) <= 0) {
            close(s);
            cout << "No response from " << ip << endl;
            continue;
        }
        cout << "Found sensor at " << ip << ": " << response << endl;
        found_devices.push_back(new Device(ip, port, s, response));
        cout << "----------------------------" << endl;
    }
    cout << "Scan complete. Found " << found_devices.size() << " device(s)." << endl;
    return found_devices;
}