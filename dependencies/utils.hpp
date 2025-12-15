#ifndef UTILS_HPP
#define UTILS_HPP

#include "framework.hpp"
#include "streamtransmission.hpp"

void trim(char* s);
ssize_t send_message(int sockfd, std::string &message);
ssize_t recv_message(int sockfd, std::string& buf);

#endif // UTILS_HPP