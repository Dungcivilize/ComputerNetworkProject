#pragma once

#include "../framework.hpp"
#include "../utils.hpp"

static void handle_connect(const string& received, string& reply, bool& closefd)
{
    string code, flag, appid, pass;
    stringstream ss(received);
    if (!(ss >> appid >> pass))
    {
        code = to_string(ERROR_MISSING_ARG);
        flag = "ERR";
        appid = "NaN";
        reply = "CONNECT " + code + " Expected AppID and Password";
        closefd = true;
    }
    else if (pass != password)
    {
        code = to_string(ERROR_PW_INCORRECT);
        flag = "ERR";
        reply = "CONNECT " + code + " Incorrect password";
        closefd = true;
    }
    else
    {
        code = to_string(SUCCESS_CONNECTION);
        flag = "OK";
        string token = randstr(TOKEN_SIZE);
        credentials[token] = appid;
        reply = "CONNECT " + code + " " + token + " " + id + " " + type;
    }
    string log = "CONNECT " + flag + ":" + code + " " + appid;
    logging(PLOG, log);
}