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

static void execute_fertilize(Device& device, int c, int v)
{
    int code = 0;
    string resp;
    if (fertilize(device, c, v, resp, code))
        cout << resp + ". Status code: " + to_string(code) << endl;
    else
        cerr << "Error: " + resp + ". Status code: " + to_string(code) << endl;
}