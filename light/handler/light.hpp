#pragma once

#include "../framework.hpp"
#include "../utils.hpp"
#include "../work.hpp"

static void handle_light(const string& received, string& reply)
{
    string flag, code, appid, token;
    int p, s;
    stringstream ss(received);
    if (!(ss >> token >> p >> s))
    {
        code = to_string(ERROR_MISSING_ARG);
        flag = "ERR";
        appid = "NaN";
        reply = "LIGHT " + code + " Expected credential token, P, S";
    }
    else if (!check_credential(token))
    {
        code = to_string(ERROR_INVALID_TOKEN);
        flag = "ERR";
        appid = "NaN";
        reply = "LIGHT " + code + " Invalid token";
    }
    else if (p < 0 || s < 0)
    {
        code = to_string(ERROR_INVALID_VALUE);
        flag = "ERR";
        appid = credentials[token];
        reply = "LIGHT " + code + " Invalid value";
    }
    {
        code = to_string(SUCCESS_COMMAND);
        flag = "OK";
        appid = credentials[token];
        reply = "LIGHT " + code + " Lights ON";
        int p_in = p ? p : P;
        int s_in = s ? s : S;
        pthread_mutex_lock(&m);  
        work(p_in, s_in);
        pthread_cond_signal(&cnd);
        pthread_mutex_unlock(&m);
    }
    string log = "LIGHT " + flag + ":" + code + " " + appid;
    logging(PLOG, log);
}