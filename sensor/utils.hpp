#pragma once

#include "framework.hpp"

static bool parse_message(const string& message, string& opcode, string& content)
{
    stringstream ss(message);
    if (!(ss >> opcode >> std::ws)) return false;
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