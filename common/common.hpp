#pragma once
#include <string>

int check_args(int argc, char* argv[]);
void call_api(int sockfd, std::string request);