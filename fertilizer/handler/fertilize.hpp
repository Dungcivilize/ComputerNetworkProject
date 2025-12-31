#pragma once

#include "../framework.hpp"
#include "../utils.hpp"
#include "../work.hpp"

static void handle_fertilize(const string& received, string& reply)
{
    string flag, code, appid, token;
    int c = 0, v = 0;
    stringstream ss(received);
    if (!(ss >> token >> c >> v))
    {
        code = to_string(ERROR_MISSING_ARG);
        flag = "ERR";
        appid = "NaN";
        reply = "FERTILIZE " + code + " Expected credential token";
    }
    else if (!check_credential(token))
    {
        code = to_string(ERROR_INVALID_TOKEN);
        flag = "ERR";
        appid = "NaN";
        reply = "FERTILIZE " + code + " Invalid token";
    }
    else if (v < 0)
    {
        code = to_string(ERROR_INVALID_VALUE);
        flag = "ERR";
        appid = credentials[token];
        reply = "FERTILIZE " + code + " Invalid value";
    }
    else
    {
        code = to_string(SUCCESS_COMMAND);
        flag = "OK";
        appid = credentials[token];
        int C_in = c ? c : C;
        int V_in = v ? v : V;
        reply = "FERTILIZE " + code + " Fertilizing with C=" + to_string(C_in) + " V=" + to_string(V_in);
        pthread_mutex_lock(&m);
        work(C_in, V_in);
        pthread_cond_signal(&cnd);
        pthread_mutex_unlock(&m);
    }
    string log = "FERTILIZE " + flag + ":" + code + " " + appid;
    logging(PLOG, log);
}