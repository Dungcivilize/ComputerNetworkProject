#pragma once

#include <string>
#include <sstream>
#include <vector>

using namespace std;

class ClientInfo; // must be defined before including this header in sensor.hpp
class SensorDataStructure;

bool requires_authentication(std::string token, std::stringstream& ss, int clientfd);
bool read_from_ss(std::stringstream& ss, int clientfd, std::vector<std::string>& out, int count, std::string response_on_fail = "3");
void handle_control_commands(int clientfd, std::stringstream& ss, std::string type = "sensor");
std::string generate_token(std::string name, size_t bytes = 16);
void handle_connect(std::string peer_ip, ClientInfo* info, std::stringstream& ss, std::string pass, std::string token);
void handle_control(std::string peer_ip, SensorDataStructure data, int clientfd, std::stringstream& ss);
