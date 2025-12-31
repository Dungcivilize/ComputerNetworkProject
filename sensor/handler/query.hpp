#pragma once

#include "../framework.hpp"
#include "../utils.hpp"

static void handle_query(const string& received, string& reply)
{
    string code, flag, token, scope, appid;
    stringstream ss(received);
    if (!(ss >> token >> scope))
    {
        code = to_string(ERROR_MISSING_ARG);
        flag = "ERR";
        appid = "NaN";
        reply = "QUERY " + code + " Expected Credential token, scope";
    }
    else if (!check_credential(token))
    {
        code = to_string(ERROR_INVALID_TOKEN);
        flag = "ERR";
        appid = "NaN";
        reply = "QUERY " + code + " Invalid credential token";
    }
    else if (scope != "STATE" && scope != "PARAM" && scope != "SCHEDULE" && scope != "ALL")
    {
        code = to_string(ERROR_INVALID_SCOPE);
        flag = "ERR";
        appid = credentials[token];
        reply = "QUERY " + code + " Invalid scope. Expected either STATE, PARAM, SCHEDULE, ALL";
    }
    else
    {
        code = to_string(SUCCESS_QUERY);
        flag = "OK";
        appid = credentials[token];
        string message, display_schedule;
        if (scope == "STATE") message = to_string(power);
        else if (scope == "PARAM") message = to_string(T);
        else if (scope == "SCHEDULE") message = "NaN";
        else message = to_string(power) + " " + to_string(T) + " " + "NaN";
        reply = "QUERY " + code + " " + message;
    }
    string extra = flag == "OK" ? scope : "";
    string log = "QUERY " + flag + ":" + code + " " + appid + " " + extra;
    logging(PLOG, log);
}