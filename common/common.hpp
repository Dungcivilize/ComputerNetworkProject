#pragma once
#include <string>
#include <vector>
#include "dataStructure.hpp"

int check_args(int argc, char* argv[]);
Device* listDeviceToSelect(std::vector<Device*>& devices, bool accept_disconnected = false);
std::string convert_time_t_to_string_only_time(time_t raw_time);
time_t convert_string_only_time_to_time_t(const std::string& time_str);