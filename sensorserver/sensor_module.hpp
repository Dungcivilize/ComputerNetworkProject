#pragma once
#include <string>
#include "../dependencies/index.hpp"
#include "sensorDataStructure.hpp"

// forward declare ClientInfo to avoid circular include with sensor.hpp
class ClientInfo;

bool requires_authentication(std::string token, stringstream& ss, int clientfd) {
    string inp_token;
    if (!(ss >> inp_token)) {
        send_message(clientfd, "2");
        return false;
    }
    if (inp_token != token) {
        send_message(clientfd, "2");
        return false;
    }
    return true;
}

bool read_from_ss(stringstream& ss, int clientfd, std::vector<std::string>& out, int count, std::string response_on_fail) {
    out.clear();
    for (int i = 0; i < count; ++i) {
        std::string temp;
        if (!(ss >> temp)) {
            send_message(clientfd, response_on_fail);
            return false;
        }
        out.push_back(temp);
    }
    return true;
}

void handle_control_commands(int clientfd, stringstream& ss, string type)
{
    string response;
    if (type == "sprinkler") {
        int volume;
        ss >> volume;
    } else {
        // reply OK (generic)
        response = "300";
        send_message(clientfd, response);        
    }
}

// Generate a token derived from sensor name and random entropy
string generate_token(string name, size_t bytes)
{
    // seed RNG with combination of name hash and random_device
    size_t name_hash = std::hash<string>{}(name);
    random_device rd;
    uint32_t seed = (uint32_t)(name_hash ^ rd());
    mt19937_64 rng(seed);

    // produce `bytes` random bytes and return hex string prefixed by name
    std::ostringstream oss;
    oss << name << "-";
    for (size_t i = 0; i < bytes; ++i)
    {
        uint8_t b = (uint8_t)(rng() & 0xFF);
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    }
    return oss.str();
}

void handle_connect(string peer_ip, ClientInfo* info, stringstream& ss, string pass, string &token) {
    cout << "Received connect request from client " << peer_ip << endl;
    vector<string> params;
    string response;
    if (!read_from_ss(ss, info->clientfd, params, 2, "3")) {
        return;
    }
    int app_id_parsed = stoi(params[0]);
    string inp_pass = params[1];
    cout << "  app_id=" << app_id_parsed << " password=<redacted>" << endl;
    if (info->app_id == app_id_parsed)
    {
        response = "201";
        send_message(info->clientfd, response);
        return;
    }
    if (inp_pass == pass)
    {
        info->app_id = app_id_parsed;
        // generate token now and return in format: 2 200 <token>
        token = generate_token(info->self->name);
        response = "200 " + token;
        send_message(info->clientfd, response);
    }
    else
    {
        response = "202";
        send_message(info->clientfd, response);
    }
}
void handle_control(string peer_ip, SensorDataStructure& data, int clientfd, stringstream& ss) {
    vector<string> params;
    string response;
    if (!(read_from_ss(ss, clientfd, params, 1))) {
        return;
    }
    string action = params[0];
    cout << "Received control request from client " << peer_ip << ": ";
    if (action == "POWER_ON" || action == "POWER_OFF" || action == "0" || action == "1") {
        string state = (action == "POWER_ON" || action == "1") ? "ON" : "OFF";
        cout << "power control -> set state to " << state << endl;
        if (data.powered_on == true && state == "ON") {
            response = "311";
            send_message(clientfd, response);
            return;
        }
        if (data.powered_on == false && state == "OFF") {
            response = "312";
            send_message(clientfd, response);
            return;
        } 
        data.powered_on = (state == "ON");
        response = "310 " + to_string(data.calculate_power()) + " " + (data.powered_on ? "1" : "0");
        send_message(clientfd, response);
    } else if (action == "TIMER" || action == "3") {
        if (!read_from_ss(ss, clientfd, params, 2)) {
            return;
        }
        string state = params[0];
        int minutes = stoi(params[1]);

        // Kiểm tra dữ liệu hợp lệ
        if (state != "0" && state != "1") {
            response = "3";
            send_message(clientfd, response);
            return;
        }
        if (minutes < 0) {
            response = "3";
            send_message(clientfd, response);
            return;
        }
        // Nếu current_time nhỏ hơn thời gian đã đặt thì báo đã đặt hẹn giờ
        time_t current_time = time(nullptr);
        if (data.timer_time > current_time && data.current_action == "TIMER") {
            response = "351";
            send_message(clientfd, response);
            return;
        }
        // Nếu trạng thái hẹn giờ trùng với trạng thái hiện tại thì báo lỗi
        if (data.powered_on == true && state == "1") {
            response = "352";
            send_message(clientfd, response);
            return;
        }
        if (data.powered_on == false && state == "0") {
            response = "353";
            send_message(clientfd, response);
            return;
        }
        
        // Tính thời điểm thực hiện hành động
        time_t action_time = current_time + minutes * 60;
        data.timer_set_to = (state == "1");
        data.timer_time = action_time;
        data.current_action = "TIMER";
        cout << "timer control -> set to " << (data.timer_set_to ? "ON" : "OFF") << " in " << minutes << " minutes" << endl;
        response = "350 " + to_string(data.calculate_power()) + " " + to_string(minutes);
        send_message(clientfd, response);
    } else if (action == "CANCEL" || action == "4") {
        if (data.current_action == "NONE") {
            response = "361";
            send_message(clientfd, response);
            return;
        }
        cout << "cancel control request" << endl;
        response = "360 " + data.current_action;
        send_message(clientfd, response);
        data.current_action = "NONE";
    } else {
        handle_control_commands(clientfd, ss);
    }
}