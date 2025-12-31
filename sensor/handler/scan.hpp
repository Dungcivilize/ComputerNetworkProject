#pragma once

#include "../framework.hpp"
#include "../utils.hpp"

static void handle_scan(const string& received, string& reply)
{
    string code, flag;
    if (received.empty())
    {
        code = to_string(ERROR_MISSING_ARG);
        flag = "ERR";
        reply = "SCAN " + code + " Expected AppID";
    }
    else
    {
        code = to_string(SUCCESS_SCAN);
        flag = "OK";
        reply = "SCAN " + code + " " + id + " " + type;
    }
    string log = "SCAN " + flag + ":" + code + " " + received;
    logging(PLOG, log);
}