#pragma once

#include "framework.hpp"
#include "streamtransmission.hpp"

static vector<string> parse_info_message(const string& message)
{
    istringstream ss(message);
    vector<string> tokens;
    string token;

    while (ss >> token)
        tokens.push_back(token);

    return tokens;
}

static bool parse_message(const string& message, string& cmd, int& exec_code, string& content)
{
    stringstream ss(message);
    if (!(ss >> cmd >> exec_code))
        return false;
    ss >> std::ws;
    std::getline(ss, content);
    return true;
}

static bool logging(const string& filepath, const string& content)
{
    ofstream out(filepath, std::ios::app);
    if (!out)
    {
        cerr << "An error occured while trying to write into " + filepath << endl;
        return false;
    }
    time_t now = time(nullptr);
    struct tm tm;
    localtime_r(&now, &tm);

    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    string timestamp(buf);

    out << timestamp + " " + content << endl;
    out.close();
    return true;
}

static string randstr(size_t len)
{
    const char alphabet[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t alphabet_len = sizeof(alphabet) - 1;

    mt19937 rng(random_device{}());
    uniform_int_distribution<size_t> dist(0, alphabet_len - 1);

    string s;
    s.reserve(len);
    for (size_t i = 0; i < len; i++)
        s.push_back(alphabet[dist(rng)]);
    return s;
}

template <class Ty>
static ssize_t find_by_id(vector<Ty>& list, const string& id)
{
    try
    {
        for (size_t idx = 0; idx < list.size(); idx++)
            if (list[idx].id == id)
                return (ssize_t)idx;
        return -1;
    }
    catch (invalid_argument e)
    {
        cerr << "Generic class passed in does not contain id field: " + string(e.what()) << endl;
        return -1;
    }
}

static bool communicate(int fd, const string& opcode, string buf, string& resp, int& exec_code)
{
    bool flag = true;
    if (!send_message(fd, buf))
    {
        exec_code = ERROR_NO_CONNECTION;
        resp = "Cannot send request to the device";
        flag = false;
    }
    if (!recv_message(fd, buf))
    {
        exec_code = ERROR_NO_CONNECTION;
        resp = "Cannot receive response from the device";
        flag = false;
    }
    
    string cmd;
    if (!parse_message(buf, cmd, exec_code, resp))
    {
        exec_code = ERROR_BAD_REQUEST;
        resp = "Failed to parse received response";
        flag = false;
    }
    if (cmd != opcode)
    {
        exec_code = ERROR_BAD_REQUEST;
        resp = "Action does not match what requested";
        flag = false;
    }

    return true;
}