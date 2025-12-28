#pragma once



#include "framework.hpp"
#include "device.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

static bool query(Device& device, string& resp, int& exec_code)
{
    string buf = "QUERY " + device.token;
    
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
    if (cmd != "QUERY")
    {
        exec_code = ERROR_BAD_REQUEST;
        resp = "Response does not match the expected format for this protocol";
        return false;
    }
    ss >> exec_code;
    auto p1 = buf.find(' ');
    auto p2 = buf.find(' ', p1 + 1);
    resp = buf.substr(p2 + 1);
    
    if (exec_code != SUCCESS_QUERY) return false;

    vector<string> tokens = parse_info_message(resp);
    vector<int> params;
    for (auto& token : tokens)
        params.push_back(stoi(token));
    
    device.set_params(params);
    return true;
}

static void execute_query(vector<Device>& connected, const string& id)
{
    ssize_t index = find_by_id(connected, id);
    if (index == -1)
        cerr << "Error: You have not connected to this device yet." << endl;
    {
        int code = 0;
        string resp;
        if (query(connected[index], resp, code))
            cout << "Query success. Status code: " + to_string(code) + "\nDevice info:\n" + connected[index].display() << endl;
        else
            cerr << "Error: " + resp + ". Status code: " + to_string(code) << endl;
    }
}