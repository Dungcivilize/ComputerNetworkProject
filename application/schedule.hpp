#pragma once

#include "framework.hpp"
#include "device.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

static bool schedule(Device& device, const string& scope, const string& timestamp, string& resp, int& exec_code)
{
    string buf = "SCHEDULE " + device.token + " " + scope + " " + timestamp;
    communicate(device.fd, "SCHEDULE", buf, resp, exec_code);
    string message = string("SCHEDULE ") + (exec_code == SUCCESS_COMMAND ? "OK:" : "ERR:") + to_string(exec_code) + " " + resp;
    logging(PLOG, message);
    if (exec_code != SUCCESS_COMMAND) return false;

    if (scope == "ADD")
        device.schedule.push_back(timestamp);
    else
    {
        auto it = find(device.schedule.begin(), device.schedule.end(), timestamp);
        if (it != device.schedule.end()) device.schedule.erase(it);
    }
    return true;
}

static void execute_schedule(vector<Device>& devices, const string& id, const string& scope, const string& timestamp)
{
    ssize_t index = find_by_id(devices, id);
    if (index == -1)
        cerr << "Error: You have not connected to this device yet." << endl;
    else
    {
        int code = 0;
        string resp;
        if (schedule(devices[index], scope, timestamp, resp, code))
            cout << "Update schedule success. Status code: " + to_string(code) << endl;
        else
            cerr << "Error: " + resp + ". Status code: " + to_string(code) << endl;
    }
}