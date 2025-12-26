#pragma once

#include "framework.hpp"

static vector<string> parse_info_message(const string& message)
{
    stringstream ss(message);
    string buffer;
    vector<string> tokens;

    while (std::getline(ss, buffer, ':'))
    {
        if (buffer.empty()) return vector<string>();
        tokens.push_back(buffer);
    }
    
    return tokens;
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