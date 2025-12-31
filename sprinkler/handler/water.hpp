#pragma once

#include "../framework.hpp"
#include "../utils.hpp"
#include "../work.hpp"

static void handle_water(const string& received, string& reply)
{
    string flag, code, appid, token;
    int v = 0;
    stringstream ss(received);
    if (!(ss >> token >> v))
    {
        code = to_string(ERROR_MISSING_ARG);
        flag = "ERR";
        appid = "NaN";
        reply = "WATER " + code + " Expected credential token";
    }
    else if (!check_credential(token))
    {
        code = to_string(ERROR_INVALID_TOKEN);
        flag = "ERR";
        appid = "NaN";
        reply = "WATER " + code + " Invalid token";
    }
    else if (v < 0)
    {
        code = to_string(ERROR_INVALID_VALUE);
        flag = "ERR";
        appid = credentials[token];
        reply = "WATER " + code + " Invalid value";
    }
    else
    {
        code = to_string(SUCCESS_COMMAND);
        flag = "OK";
        appid = credentials[token];
        int V_in = v ? v : V;
        reply = "WATER " + code + " Watering with V=" + to_string(V_in);
        pthread_mutex_lock(&m);
        work(V_in);
        pthread_cond_signal(&cnd);
        pthread_mutex_unlock(&m);
    }
    string log = "WATER " + flag + ":" + code + " " + appid;
    logging(PLOG, log);
}