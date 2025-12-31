#pragma once

#include "../framework.hpp"
#include "../utils.hpp"

static void handle_change_password(const string& received, string& reply)
{
    string code, flag, token, current_pass, new_pass, appid;
    stringstream ss(received);
    if (!(ss >> token >> current_pass >> new_pass))
    {
        code = to_string(ERROR_MISSING_ARG);
        flag = "ERR";
        appid = "NaN";
        reply = "CHANGE_PW " + code + " Expected Credential token, current password, new password";
    }
    else if (!check_credential(token))
    {
        code = to_string(ERROR_INVALID_TOKEN);
        flag = "ERR";
        appid = "NaN";
        reply = "CHANGE_PW " + code + " Invalid credential token";
    }
    else if (current_pass != password)
    {
        code = to_string(ERROR_PW_INCORRECT);
        flag = "ERR";
        appid = credentials[token];
        reply = "CHANGE_PW " + code + " Incorrect password";
    }
    else
    {
        code = to_string(SUCCESS_PW_CHANGE);
        flag = "OK";
        appid = credentials[token];
        password = new_pass;
        reply = "CHANGE_PW " + code + " Password changed";
    }
    string log = "CHANGE_PW " + flag + ":" + code + " " + appid;
    logging(PLOG, log);
}