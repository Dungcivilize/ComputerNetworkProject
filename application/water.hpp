#pragma once

#include "framework.hpp"
#include "device.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

static bool water(Device& device, int v, string& resp, int& exec_code)
{
    string buf = "WATER " + device.token + " " + to_string(v);
    communicate(device.fd, "WATER", buf, resp, exec_code);
    string message = string("WATER ") + (exec_code == SUCCESS_COMMAND ? "OK:" : "ERR:") + to_string(exec_code) + " " + resp;
    logging(PLOG, message);
    return exec_code == SUCCESS_COMMAND;
}

static void execute_water(Device& device, int v)
{
    int code = 0;
    string resp;
    if (water(device, v, resp, code))
        cout << "Watering. Status code: " + to_string(code) << endl;
    else
        cerr << "Error: " + resp + ". Status code: " + to_string(code) << endl;
}