#include <iostream>
#include "common.hpp"
#include <sstream>
#include "dataStructure.hpp"
#include "../dependencies/index.hpp"
#define BUFFER_SIZE 32787

using namespace std;

int check_args(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return -1;
    }
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        printf("Invalid port. Port must be between 0 and 65535\n");
        return -1;
    }
    return port;
}

Device* listDeviceToSelect(vector<Device*>& devices, bool accept_disconnected) {
    if (devices.empty()) {
        cout << "No devices connected." << endl;
        return NULL;
    }
    cout << "Available devices:" << endl;
    for (size_t i = 0; i < devices.size(); ++i) {
        DeviceInfo info = devices[i]->info;
        cout << i << ". ID: " << info.id << ", Name: " << info.name << " (" << info.addr_str << ")" << (info.token.empty() ? " [Not Connected]" : "[Connected]") << endl;
    }
    cout << "Select device index (-1 to cancel): ";
    int index;
    cin >> index;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (index < 0 || static_cast<size_t>(index) >= devices.size()) {
        return NULL;
    }
    if (!accept_disconnected && devices[index]->info.token.empty()) {
        cout << " Device not connected properly." << endl;
        return NULL;
    }
    return devices[index];
}