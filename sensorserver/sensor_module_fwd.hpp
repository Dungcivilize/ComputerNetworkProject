#pragma once

#include <string>
#include <sstream>
#include <vector>

using namespace std;

class ClientInfo; // must be defined before including this header in sensor.hpp
class SensorDataStructure;

bool requires_authentication(std::string token, std::stringstream& ss, int clientfd, std::string id);
bool read_from_ss(std::stringstream& ss, int clientfd, std::vector<std::string>& out, int count, std::string response_on_fail, std::string id);
void handle_control_commands(int clientfd, std::stringstream& ss, SensorDataStructure* data, std::string id);
std::string generate_token(std::string name, size_t bytes = 16);
void handle_connect(ClientInfo* info, std::stringstream& ss, std::string pass, std::string &token);
void handle_control(std::string client_ip, SensorDataStructure* data, int clientfd, std::stringstream& ss, std::string id);
bool update_sensor_configuration(SensorDataStructure* data, int param_index, const std::string& param_value);