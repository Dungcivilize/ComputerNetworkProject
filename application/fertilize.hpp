#pragma once

#include "framework.hpp"
#include "device.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

static bool fertilize(Device& device, int c, int v, string& resp, int& exec_code)
{
    string buf = "FERTILIZE " + device.token + " " + to_string(c) + " " + to_string(v);
    communicate(device.fd, "FERTILIZE", buf, resp, exec_code);
    string message = string("FERTILIZE ") + (exec_code == SUCCESS_COMMAND ? "OK:" : "ERR:") + to_string(exec_code) + " " + resp;
    logging(PLOG, message);
    return exec_code == SUCCESS_COMMAND;
}

static void execute_fertilize(vector<Device>& connected, const string& device_id, int c, int v)
{
    size_t index = find_by_id(connected, device_id);
    if (index == -1)
        cerr << "Error: You have not connected to this device yet." << endl;
    else
    {
        int code = 0;
        string resp;
        if (fertilize(connected[index], c, v, resp, code))
            cout << resp + ". Status code: " + to_string(code) << endl;
        else
            cerr << "Error: " + resp + ". Status code: " + to_string(code) << endl;
    }
}