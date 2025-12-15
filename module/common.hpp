#pragma once

int check_args(int argc, char* argv[]);
void call_api(int sockfd, const std::string& request);