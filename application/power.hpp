#pragma once

#include "framework.hpp"
#include "device.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

static bool power(Device& device, const string& mode, string& resp, int& exec_code)
{
    string buf = "POWER " + device.token + " " + mode;
    communicate(device.fd, "POWER", buf, resp, exec_code);
    string message = string("POWER ") + (exec_code == SUCCESS_COMMAND ? "OK:" : "ERR:") + to_string(exec_code) + " " + resp;
    logging(PLOG, message);
    if (exec_code != SUCCESS_COMMAND) return false;
    device.state = mode == "ON" ? 1 : 0;
    return true;
}

static void execute_power(vector<Device>& connected, const string& device_id, const string& mode)
{
    size_t index = find_by_id(connected, device_id);
    if (index == -1)
        cerr << "Error: You have not connected to this device yet." << endl;
    else
    {
        int code = 0;
        string resp;
        if (power(connected[index], mode, resp, code))
            cout << "Set power successful. Status code: " + to_string(code) << endl;
        else
            cerr << "Error: " + resp + ". Status code: " + to_string(code) << endl;
    }
}