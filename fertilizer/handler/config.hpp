#pragma once

#include "../framework.hpp"
#include "../utils.hpp"

static void handle_config(const string& received, string& reply)
{
    string code, flag, appid, token, param;
    int value;
    stringstream ss(received);
    
    if (!(ss >> token >> param >> value))
    {
        code = to_string(ERROR_MISSING_ARG);
        flag = "ERR";
        appid = "NaN";
        reply = "CONFIG " + code + " Expected credential token, param, value";
    }
    else if (!check_credential(token))
    {
        code = to_string(ERROR_INVALID_TOKEN);
        flag = "ERR";
        appid = "NaN";
        reply = "CONFIG " + code + " Invalid credential token";
    }
    else if (param != "C" && param != "V" && param != "Nmin" && param != "Pmin" && param != "Kmin")
    {
        code = to_string(ERROR_INVALID_PARAM);
        flag = "ERR";
        appid = credentials[token];
        reply = "CONFIG " + code + " Unknown parameter received";
    }
    else if (value <= 0)
    {
        code = to_string(ERROR_INVALID_VALUE);
        flag = "ERR";
        appid = credentials[token];
        reply = "CONFIG " + code + " Invalid value received";
    }
    else
    {
        code = to_string(SUCCESS_CONFIG);
        flag = "OK";
        appid = credentials[token];
        pthread_mutex_lock(&m);
        if (param == "C") C = value;
        else if (param == "V") V = value;
        else if (param == "Nmin") Nmin = value;
        else if (param == "Pmin") Pmin = value;
        else Kmin = value;
        pthread_cond_signal(&cnd);
        pthread_mutex_unlock(&m);
        reply = "CONFIG " + code + " " + param + " has been set to " + to_string(value);
    }
    string extra = flag == "OK" ? param + "=" + to_string(value) : "";
    string log = "CONFIG " + flag + ":" + code + " " + appid + " " + extra;
    logging(PLOG, log);
}