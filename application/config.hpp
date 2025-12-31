#include "framework.hpp"
#include "streamtransmission.hpp"
#include "device.hpp"
#include "utils.hpp"

static bool config(Device& device, const string& param, int value, string& resp, int& exec_code)
{
    string buf = "CONFIG " + device.token + " " + param + " " + to_string(value);
    communicate(device.fd, "CONFIG", buf, resp, exec_code);
    string message = string("CONFIG ") + (exec_code == SUCCESS_CONFIG ? "OK:" : "ERR:") + to_string(exec_code) + " " + resp;
    logging(PLOG, message);

    if (exec_code != SUCCESS_CONFIG) return false;

    if (device.type == "sensor" || (device.type == "light" && param == "P") || (device.type == "sprinkler" && param == "Hmin") || (device.type == "fertilizer" && param == "C"))
        device.params[0] = value;
    else if ((device.type == "sprinkler" && param == "Hmax") || (device.type == "fertilizer" && param == "V") || (device.type == "LIGHT" && param == "S"))
        device.params[1] = value;
    else if ((param == "Nmin" && device.type == "fertilizer") || (device.type == "sprinkler" && param == "V"))
        device.params[2] = value;
    else if (param == "Pmin")
        device.params[3] = value;
    else
        device.params[4] = value;

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