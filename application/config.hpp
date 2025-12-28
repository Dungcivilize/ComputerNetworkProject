#include "framework.hpp"
#include "streamtransmission.hpp"
#include "device.hpp"
#include "utils.hpp"

static bool config(Device& device, const string& param, int value, string& resp, int& exec_code)
{
    string buf = "CONFIG " + device.token + " " + param + " " + to_string(value);
    
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
    if (cmd != "CONFIG")
    {
        exec_code = ERROR_BAD_REQUEST;
        resp = "Response does not match the expected format of this protocol";
        return false;
    }

    ss >> exec_code;
    auto p1 = buf.find(' ');
    auto p2 = buf.find(' ', p1 + 1);
    resp = buf.substr(p2 + 1);

    if (exec_code != SUCCESS_CONFIG) return false;

    if (device.type == "sensor" || device.type == "light" || (device.type == "sprinkler" && param == "HMIN") || (device.type == "fertilizer" && param == "C"))
        device.params[1] = value;
    else if ((device.type == "sprinkler" && param == "HMAX") || (device.type == "fertilizer" && param == "V"))
        device.params[2] = value;
    else if (param == "NMIN")
        device.params[3] = value;
    else if (param == "PMIN")
        device.params[4] = value;
    else
        device.params[5] = value;

    return true;
}

static void execute_config(vector<Device>& devices, const string& id, const string& param, const int value)
{
    ssize_t index = find_by_id(devices, id);
    if (index == -1)
        cerr << "Error: You have not connected to this device yet." << endl;
    else
    {
        int code = 0;
        string resp;
        if (config(devices[index], param, value, resp, code))
            cout << "Config success. Status code: " + to_string(code) << endl;
        else
            cerr << "Error: " + resp + ". Status code: " + to_string(code) << endl;
    }
}