#pragma once

#include "../framework.hpp"
#include "../utils.hpp"

static void handle_power(const string& received, string& reply)
{
    string flag, code, token, state, appid;
    stringstream ss(received);
    if (!(ss >> token >> state))
    {
        code = to_string(ERROR_MISSING_ARG);
        flag = "ERR";
        appid = "NaN";
        reply = "POWER " + code + " Expected credential token, state";
    }
    else if (!check_credential(token))
    {
        code = to_string(ERROR_INVALID_TOKEN);
        flag = "ERR";
        appid = "NaN";
        reply = "POWER " + code + " Invalid token";
    }
    else if ((state == "ON" && power) || (state == "OFF" && !power) || (state != "ON" && state != "OFF"))
    {
        code = to_string(ERROR_INVALID_VALUE);
        flag = "ERR";
        appid = credentials[token];
        reply = "POWER " + code + " Invalid state";
    }
    else
    {
        code = to_string(SUCCESS_COMMAND);
        flag = "OK";
        appid = credentials[token];
        pthread_mutex_lock(&m);
        power = state == "ON" ? 1 : 0;
        pthread_cond_signal(&cnd);
        pthread_mutex_unlock(&m);
        reply = "POWER " + code + " OK";
    }
    string extra = flag == "OK" ? state : "";
    string log = "POWER " + flag + ":" + code + " " + appid + " " + extra;
    logging(PLOG, log);
}