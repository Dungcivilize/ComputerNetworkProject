#pragma once

#include "framework.hpp"
#include "streamtransmission.hpp"
#include "device.hpp"
#include "utils.hpp"
#include "query.hpp"

bool parse_connection_reply(const string& resp, string& token, string& id, string& type)
{
    vector<string> tokens = parse_info_message(resp);
    if (tokens.empty()) return false;
    for (size_t idx = 0; idx < tokens.size(); idx++)
        switch (idx)
        {
        case 0:
            token = tokens[idx];
            break;
        case 1:
            id = tokens[idx];
            break;
        case 2:
            type = tokens[idx];
            break;
        }
    return true;
}

static bool try_connection(int fd, const string& id, const string& password, string& resp, int& exec_code)
{
    string buf = "CONNECT " + id + " " + password;

    if (!send_message(fd, buf))
    {
        exec_code = ERROR_NO_CONNECTION;
        resp = "Cannot send request to the device";
        return false;
    }
    if (!recv_message(fd, buf))
    {
        exec_code = ERROR_NO_CONNECTION;
        resp = "Cannot receive response from the device";
        return false;
    }

    stringstream ss(buf);
    string cmd;
    ss >> cmd;
    if (cmd != "CONNECT")
    {
        exec_code = ERROR_BAD_REQUEST;
        resp = "Response does not match the expected format for this protocol";
        return false;
    }

    ss >> exec_code;
    auto p1 = buf.find(' ');
    auto p2 = buf.find(' ', p1 + 1);
    resp = buf.substr(p2 + 1);
    
    return exec_code == SUCCESS_CONNECTION;
}

static bool connection(DeviceInfo& available, const string& id, const string& password, string& resp, int& exec_code, vector<Device>& connected)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket");
        exec_code = ERROR_NO_CONNECTION;
        resp = "Cannot create socket for connecting to device";
        return false;
    }

    int rc = connect(fd, (sockaddr*)&available.addr, sizeof(available.addr));
    if (rc)
    {
        perror("connect");
        close(fd);
        exec_code = ERROR_NO_CONNECTION;
        resp = "Attempt to connect to device failed";
        return false;
    }

    bool rs = try_connection(fd, id, password, resp, exec_code);
    if (rs)
    {
        string device_id, type, token;
        if (!parse_connection_reply(resp, token, device_id, type))
        {
            close(fd);
            exec_code = ERROR_BAD_REQUEST;
            resp = "Response does not match the expected format for this protocol";
            return false;
        }
        if (device_id != available.id || type != available.type)
        {
            close(fd);
            exec_code = ERROR_BAD_REQUEST;
            resp = "Failed to validate responded device's information";
            return false;
        }
        connected.push_back(Device(fd, device_id, type, token));
        return true;
    }
    
    return false;
}

static void execute_connect(vector<DeviceInfo>& available, const string& device_id, const string& app_id, const string& password, vector<Device>& connected)
{
    size_t index = find_by_id(available, device_id);
    if (index == -1)
        cerr << "Error: Unknown device, please use scan to find this device first." << endl;
    {
        int code = 0;
        string resp;
        if (connection(available[index], app_id, password, resp, code, connected))
        {
            cout << "Connection successful. Status code: " + to_string(code) << endl;
            execute_query_after_connection(connected, connected.back().id);
        }
        else
            cerr << "Error: " + resp + ". Status code: " + to_string(code) << endl; 
    }
}