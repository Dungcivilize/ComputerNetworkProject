#pragma once

#include "framework.hpp"

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

static vector<double> randnumbers(size_t len, double min = 0, double max = 1)
{
    vector<double> stats;
    random_device rd;
    mt19937 rng(rd());
    uniform_real_distribution<> dist(min, max);
    for (int count = 0; count < len; count++)
        stats.push_back(dist(rng));
    return stats;
}