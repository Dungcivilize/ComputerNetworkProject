#pragma once

#include "framework.hpp"
#include "streamtransmission.hpp"
#include "device.hpp"
#include "utils.hpp"
#include "query.hpp"

bool parse_connection_reply(const string& resp, string& token, string& id, string& type)
{
    vector<string> tokens = parse_info_message(resp);
    if (tokens.size() != 3) return false;
    token = tokens[0];
    id = tokens[1];
    type = tokens[2];
    return true;
}

static bool try_connection(int fd, const string& id, const string& password, string& resp, int& exec_code)
{
    string buf = "CONNECT " + id + " " + password;
    communicate(fd, "CONNECT", buf, resp, exec_code);
    string message = string("CONNECT ") + (exec_code == SUCCESS_CONNECTION ? "OK:" : "ERR:") + to_string(exec_code) + " " + resp;
    logging(PLOG, message);
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
            resp = "Cannot parse received response";
            return false;
        }
        if (device_id != available.id || type != available.type)
        {
            close(fd);
            exec_code = ERROR_BAD_REQUEST;
            resp = "Cannot vailidate responded device";
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