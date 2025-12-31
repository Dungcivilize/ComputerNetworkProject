#pragma once

#include "framework.hpp"
#include "device.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

static bool query(Device& device, const string& scope, string& resp, int& exec_code)
{
    string buf = "QUERY " + device.token + " " + scope;
    communicate(device.fd, "QUERY", buf, resp, exec_code);
    string message = string("QUERY ") + (exec_code == SUCCESS_QUERY ? "OK:" : "ERR:") + to_string(exec_code) + " " + resp;
    logging(PLOG, message);
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
        else if (device.type == "light") nparams = 2;
        else if (device.type == "sprinkler") nparams = 3;
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