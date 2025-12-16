#pragma once
#include <string>
#include <vector>
#include "dataStructure.hpp"

int check_args(int argc, char* argv[]);
Device* listDeviceToSelect(std::vector<Device*>& devices, bool accept_disconnected = false);