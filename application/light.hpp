#pragma once

#include "framework.hpp"
#include "device.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

static bool light(Device& device, string& resp, int& exec_code)
{
    string buf = "LIGHT " + device.token;
    
    if (!send_message(device.fd, buf))
    {
        exec_code = ERROR_NO_CONNECTION;
        resp = "Cannot send request to device";
        return false;
    }
    if (!recv_message(device.fd, buf))
    {
        exec_code = ERROR_NO_CONNECTION;
        resp = "Cannot receive response from device";
        return false;
    }

    stringstream ss(buf);
    string cmd;
    ss >> cmd;
    if (cmd != "LIGHT")
    {
        exec_code = ERROR_BAD_REQUEST;
        resp = "Response does not match the expected format for this protocol";
        return false;
    }
    ss >> exec_code;
    auto p1 = buf.find(' ');
    auto p2 = buf.find(' ', p1 + 1);
    resp = buf.substr(p2 + 1);

    return exec_code == SUCCESS_COMMAND;
}

static void execute_light(vector<Device>& connected, const string& device_id)
{
    size_t index = find_by_id(connected, device_id);
    if (index == -1)
        cerr << "Error: You have not connected to this device yet." << endl;
    else
    {
        int code = 0;
        string resp;
        if (light(connected[index], resp, code))
            cout << "Light on. Status code: " + to_string(code) << endl;
        else
            cerr << "Error: " + resp + ". Status code: " + to_string(code) << endl;
    }
}