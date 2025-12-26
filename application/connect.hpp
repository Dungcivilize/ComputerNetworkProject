#pragma once

#include "framework.hpp"
#include "streamtransmission.hpp"
#include "device.hpp"

bool parse_connection_reply(const string& resp, string& id, string& name, string& type, string& token, vector<int>& params)
{
    vector<string> tokens = parse_info_message(resp);
    if (tokens.empty()) return false;
    for (size_t idx = 0; idx < tokens.size(); idx++)
        switch (idx)
        {
        case 0:
            id = tokens[idx];
            break;
        case 1:
            name = tokens[idx];
            break;
        case 2:
            type = tokens[idx];
            break;
        case 3:
            token = tokens[idx];
            break;
        default:
            params.push_back(stoi(tokens[idx]));
        }
    return true;
}

static bool verification(int fd, const string& id, const string& password, string& resp, int& exec_code)
{
    string buf = "CONNECT " + id + " " + password;

    if (!send_message(fd, buf))
    {
        exec_code = ERROR_NO_CONNECTION;
        return false;
    }
    if (!recv_message(fd, buf))
    {
        exec_code = ERROR_NO_CONNECTION;
        return false;
    }

    stringstream ss(buf);
    string cmd;
    ss >> cmd;
    if (cmd != "CONNECT")
    {
        exec_code = ERROR_BAD_REQUEST;
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
        return false;
    }

    int rc = connect(fd, (sockaddr*)&available.addr, sizeof(available.addr));
    if (rc)
    {
        perror("connect");
        close(fd);
        exec_code = ERROR_NO_CONNECTION;
        return false;
    }

    bool rs = verification(fd, id, password, resp, exec_code);
    if (rs)
    {
        string device_id, name, type, token;
        vector<int> params;
        if (!parse_connection_reply(resp, device_id, name, type, token, params))
        {
            close(fd);
            exec_code = ERROR_BAD_REQUEST;
            return false;
        }
        if (device_id != available.id || type != available.type)
        {
            close(fd);
            exec_code = ERROR_BAD_REQUEST;
            return false;
        }
        connected.push_back(Device(fd, device_id, name, type, token, params));
        return true;
    }
    
    return false;
}