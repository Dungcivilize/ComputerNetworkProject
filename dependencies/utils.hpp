#ifndef UTILS_HPP
#define UTILS_HPP

#include "framework.hpp"
#include "streamtransmission.hpp"

void trim(char* s);
ssize_t send_message(int sockfd, std::string &message);
ssize_t recv_message(int sockfd, std::string& buf);
void call_api(int sockfd, std::string request);
bool get_local_ipv4(std::string &out_ip);`
int connect_to(const std::string &ip, uint16_t port, int timeout_ms = 200);
#endif // UTILS_HPP