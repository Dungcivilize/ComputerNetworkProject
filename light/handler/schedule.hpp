#pragma once

#include "../framework.hpp"
#include "../utils.hpp"

static bool validate_timestamp(const string& timestamp)
{
    int h, m, s;
    int n = sscanf(timestamp.c_str(), "%d:%d:%d", &h, &m, &s);
    if (n < 3) return false;
    if (h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59) return false;
    return true;
}

static void handle_schedule(const string& received, string& reply)
{
    string flag, code, appid, token, scope, timestamp;
    stringstream ss(received);
    if (!(ss >> token >> scope >> timestamp))
    {
        code = to_string(ERROR_MISSING_ARG);
        flag = "ERR";
        appid = "NaN";
        reply = "SCHEDULE " + code + " Expected credential, token, timestamp";
    }
    if (!check_credential(token))
    {
        code = to_string(ERROR_INVALID_TOKEN);
        flag = "ERR";
        appid = "NaN";
        reply = "SCHEDULE " + code + " Invalid token";
    }
    else if (scope != "ADD" && scope != "REMOVE")
    {
        code = to_string(ERROR_INVALID_SCOPE);
        flag = "ERR";
        appid = credentials[token];
        reply = "SCHEDULE " + code + " Invalid scope";
    }
    else if (!validate_timestamp(timestamp))
    {
        code = to_string(ERROR_INVALID_VALUE);
        flag = "ERR";
        appid = credentials[token];
        reply = "SCHEDULE " + code + " Invalid time format";
    }
    else
    {
        code = to_string(SUCCESS_COMMAND);
        flag = "OK";
        appid = credentials[token];
        reply = "SCHEDULE " + code + " Schedule updated";
        pthread_mutex_lock(&m);
        if (scope == "ADD")
            schedule.push_back(timestamp);
        else
        {
            auto it = find(schedule.begin(), schedule.end(), timestamp);
            if (it != schedule.end()) schedule.erase(it);
        }
        pthread_cond_signal(&cnd);
        pthread_mutex_unlock(&m);
    }
    string extra = flag == "OK" ? scope : "";
    string log = "SCHEDULE " + flag + ":" + code + " " + appid + " " + extra;
    logging(PLOG, log);
}