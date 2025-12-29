#pragma once

#include "framework.hpp"
#include "device.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

static bool query(Device& device, const string& scope, string& resp, int& exec_code)
{
    string buf = "QUERY " + device.token + " " + scope;
    
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
    if (scope == "STATE")
        device.state = stoi(tokens[0]);
    else if (scope == "PARAM")
    {
        vector<int> params;
        for (auto& token : tokens)
            params.push_back(stoi(token));
        device.params = params;
    }
    else if (scope == "SCHEDULE" && tokens[0] != "NaN")
        device.schedule = tokens;
    else
    {
        device.state = stoi(tokens[0]);
        int nparams = 0;
        if (device.type == "sensor") nparams = 1;
        else if (device.type == "sprinkler" || device.type == "light") nparams = 2;
        else nparams = 5;
        vector<int> params;
        for (size_t idx = 1; idx <= nparams; idx++)
            params.push_back(stoi(tokens[idx]));
        device.params = params;
        if (tokens[nparams + 1] != "NaN")
        {
            vector<string> schedule;
            for (size_t idx = nparams + 1; idx < tokens.size(); idx++)
                schedule.push_back(tokens[idx]);
            device.schedule = schedule;
        }
    }
    return true;
}

static void execute_query_after_connection(vector<Device>& connected, const string& id)
{
    ssize_t index = find_by_id(connected, id);
    if (index == -1)
        cerr << "Error: You have not connected to this device yet." << endl;
    else 
    {
        int code = 0;
        string resp;
        if (query(connected[index], "ALL", resp, code))
            cout << "Device info:\n" + connected[index].display() << endl;
        else
            cerr << "Error: " + resp + ". Status code: " + to_string(code) << endl;
    }
}