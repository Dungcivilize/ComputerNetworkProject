#pragma once

#include "framework.hpp"
#include "device.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

static bool light(Device& device, int p, int s, string& resp, int& exec_code)
{
    string buf = "LIGHT " + device.token + " " + to_string(p) + " " + to_string(s);
    communicate(device.fd, "LIGHT", buf, resp, exec_code);
    string message = string("LIGHT ") + (exec_code == SUCCESS_COMMAND ? "OK:" : "ERR:") + to_string(exec_code) + " " + resp;
    logging(PLOG, message);
    return exec_code == SUCCESS_COMMAND;
}

static void execute_light(vector<Device>& connected, const string& device_id, int p, int s)
{
    size_t index = find_by_id(connected, device_id);
    if (index == -1)
        cerr << "Error: You have not connected to this device yet." << endl;
    else
    {
        int code = 0;
        string resp;
        if (light(connected[index], p, s, resp, code))
            cout << resp + ". Status code: " + to_string(code) << endl;
        else
            cerr << "Error: " + resp + ". Status code: " + to_string(code) << endl;
    }
}