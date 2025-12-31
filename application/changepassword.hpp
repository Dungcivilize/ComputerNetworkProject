#pragma once

#include "framework.hpp"
#include "device.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

static bool change_password(Device& device, const string& current_pass, const string& new_pass, string& resp, int& exec_code)
{
    string buf = "CHANGE_PW " + device.token + " " + current_pass + " " + new_pass;
    communicate(device.fd, "CHANGE_PW", buf, resp, exec_code);
    string message = string("CHANGE_PW ") + (exec_code == SUCCESS_PW_CHANGE ? "OK:" : "ERR:") + to_string(exec_code) + " " + resp;
    logging(PLOG, message);
    return exec_code == SUCCESS_PW_CHANGE;
}

static void execute_change_password(Device& device, const string& current_pass, const string& new_pass)
{
    int code = 0;
    string resp;
    if (change_password(device, current_pass, new_pass, resp, code))
        cout << "Password changed. Status code: " + to_string(code) << endl;
    else
        cerr << "Error: " + resp + ". Status code: " + to_string(code) << endl;
}