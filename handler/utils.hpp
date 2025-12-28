#include "../common/index.hpp"

ssize_t find_device_info_by_id(vector<DeviceInfo> &list, string id);
ssize_t find_device_by_id(vector<Device*> &devices, string id);
bool get_local_ipv4(string &out_ip);
int connect_to(const string &ip, uint16_t port, int timeout_ms = 200);