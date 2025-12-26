#include "framework.hpp"
#include "device.hpp"
#include "streamtransmission.hpp"

static bool change_password(Device& device, const string& current_pass, const string& new_pass, string& resp, int& exec_code)
{
    string buf = "CHANGE_PW " + device.token + " " + current_pass + " " + new_pass;
    
    if (!send_message(device.fd, buf))
    {
        exec_code = ERROR_NO_CONNECTION;
        return false;
    }
    if (!recv_message(device.fd, buf))
    {
        exec_code = ERROR_NO_CONNECTION;
        return false;
    }

    stringstream ss(buf);
    string cmd;
    ss >> cmd;
    if (cmd != "CHANGE_PW")
    {
        exec_code = ERROR_BAD_REQUEST;
        return false;
    }
    ss >> exec_code;
    auto p1 = buf.find(' ');
    auto p2 = buf.find(' ', p1 + 1);
    resp = buf.substr(p2 + 1);

    return exec_code == SUCCESS_PW_CHANGE;
}