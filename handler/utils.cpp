#include "../common/index.hpp"

ssize_t find_device_info_by_id(vector<DeviceInfo> &list, string id)
{
    for (size_t idx = 0; idx < list.size(); idx++)
        if (id == list[idx].id)
            return (ssize_t)idx;
    return -1;
}

ssize_t find_device_by_id(vector<Device*> &devices, string id)
{
    for (size_t idx = 0; idx < devices.size(); idx++)
        if (id == devices[idx]->info.id)
            return (ssize_t)idx;
    return -1;
}