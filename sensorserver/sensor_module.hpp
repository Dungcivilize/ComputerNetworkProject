#pragma once
#include <string>
#include "../dependencies/index.hpp"
#include "sensorDataStructure.hpp"

// forward declare ClientInfo to avoid circular include with sensor.hpp
class ClientInfo;

bool requires_authentication(std::string token, stringstream& ss, int clientfd, string id) {
    string inp_token;
    if (!(ss >> inp_token)) {
        send_message(clientfd, "2");
        log(id, "Client failed to provide token for authentication");
        return false;
    }
    if (inp_token != token) {
        send_message(clientfd, "2");
        log(id, "Client provided invalid token for authentication");
        return false;
    }
    return true;
}

bool read_from_ss(stringstream& ss, int clientfd, std::vector<std::string>& out, int count, std::string response_on_fail, string id) {
    out.clear();
    for (int i = 0; i < count; ++i) {
        std::string temp;
        if (!(ss >> temp)) {
            send_message(clientfd, response_on_fail);
            log(id, "Failed to read expected parameters from client");
            return false;
        }
        out.push_back(temp);
    }
    return true;
}

void handle_control_commands(int clientfd, stringstream& ss, SensorDataStructure* data, string id)
{
    std::vector<std::string> params;
    if (!data->powered_on) {
        string response = "301";
        send_message(clientfd, response);
        log(id, "Control command received but device is powered off");
        return;
    }
    string response;
    cout << "Received control request from client for " << data->type << endl;
    if (data->is_running_on_command) {
        response = "302";
        send_message(clientfd, response);
        log(id, "Control command received but device is busy running on command");
        return;
    }
    if (data->type == "sprinkler") {
        if (!read_from_ss(ss, clientfd, params, 1, "3", id)) {
            return;
        }
        int volume = stoi(params[0]);
        cout << "  sprinkler command -> water volume: " << volume << " Liters" << endl;
        if (volume < 0) {
            response = "3";
            send_message(clientfd, response);
            log(id, "Invalid parameter for sprinkler control command");
            return;
        }
        SprinklerDataStructure* sprinkler_data = dynamic_cast<SprinklerDataStructure*>(data);
        if (sprinkler_data == nullptr) {
            response = "303";
            send_message(clientfd, response);
            log(id, "Sprinkler control command received but device is not a sprinkler");
            return;
        }
        if (volume == 0) {
            volume = sprinkler_data->base_volume;
        }
        if (sprinkler_data->humidity >= sprinkler_data->max_humidity) {
            response = "321";
            send_message(clientfd, response);
            log(id, "Sprinkler control command received but humidity is already at or above maximum threshold");
            return;
        }
        if (volume > sprinkler_data->current_water_amount) {
            response = "322";
            send_message(clientfd, response);
            log(id, "Sprinkler control command received but not enough water in tank");
            return;
        }
        sprinkler_data->command_volume = volume;
        sprinkler_data->is_running_on_command = true;
        response = "320 " + to_string(sprinkler_data->calculate_power()) + " " + to_string(volume);
        send_message(clientfd, response);
        log(id, "Sprinkler control command accepted, volume: " + to_string(volume) + " Liters");
    } else if (data->type == "fertilizer") {
        int volume;
        float nitrogen_amount_per_liter;
        float phosphorus_amount_per_liter;
        float potassium_amount_per_liter;
        ss >> volume >> nitrogen_amount_per_liter >> phosphorus_amount_per_liter >> potassium_amount_per_liter;
        cout << "  fertilizer command -> water volume: " << volume << " Liters"
             << ", N: " << nitrogen_amount_per_liter << " g/L"
             << ", P: " << phosphorus_amount_per_liter << " g/L"
             << ", K: " << potassium_amount_per_liter << " g/L" << endl;
        if (volume < 0 || nitrogen_amount_per_liter < 0 || phosphorus_amount_per_liter < 0 || potassium_amount_per_liter < 0) {
            response = "3";
            send_message(clientfd, response);
            log(id, "Invalid parameter for fertilizer control command");
            return;
        }
        FertilizerDataStructure* fertilizer_data = dynamic_cast<FertilizerDataStructure*>(data);
        if (fertilizer_data == nullptr) {
            response = "303";
            send_message(clientfd, response);
            log(id, "Fertilizer control command received but device is not a fertilizer");
            return;
        }
        if (volume == 0) {
            volume = fertilizer_data->base_volume;
        }
        if (volume > fertilizer_data->current_water_amount) {
            response = "331";
            send_message(clientfd, response);
            log(id, "Fertilizer control command received but not enough water in tank");
            return;
        }
        if (nitrogen_amount_per_liter == 0) {
            nitrogen_amount_per_liter = fertilizer_data->fertilizer_amount_nitrogen_per_liter;
        }
        if (phosphorus_amount_per_liter == 0) {
            phosphorus_amount_per_liter = fertilizer_data->fertilizer_amount_phosphorus_per_liter;
        }
        if (potassium_amount_per_liter == 0) {
            potassium_amount_per_liter = fertilizer_data->fertilizer_amount_potassium_per_liter;
        }
        fertilizer_data->command_volume = volume;
        fertilizer_data->command_fertilizer_amount_nitrogen_per_liter = nitrogen_amount_per_liter;
        fertilizer_data->command_fertilizer_amount_phosphorus_per_liter = phosphorus_amount_per_liter;
        fertilizer_data->command_fertilizer_amount_potassium_per_liter = potassium_amount_per_liter;
        fertilizer_data->is_running_on_command = true;
        response = "330 " + to_string(fertilizer_data->calculate_power()) + " " + to_string(volume) + " "
                   + to_string(nitrogen_amount_per_liter) + " " + to_string(phosphorus_amount_per_liter) + " " + to_string(potassium_amount_per_liter);
        send_message(clientfd, response);
        log(id, "Fertilizer control command accepted, volume: " + to_string(volume) + " Liters"
            + ", N: " + to_string(nitrogen_amount_per_liter) + " g/L"
            + ", P: " + to_string(phosphorus_amount_per_liter) + " g/L"
            + ", K: " + to_string(potassium_amount_per_liter) + " g/L");
    } else if (data->type == "lighting") {
        if (!read_from_ss(ss, clientfd, params, 2, "3", id)) {
            return;
        }
        int duration = stoi(params[0]);
        int light_power = stoi(params[1]);
        if (light_power < 0 || duration <= 0) {
            response = "3";
            send_message(clientfd, response);
            log(id, "Invalid parameter for lighting control command");
            return;
        }
        LightingDataStructure* lighting_data = dynamic_cast<LightingDataStructure*>(data);
        if (lighting_data == nullptr) {
            response = "303";
            send_message(clientfd, response);
            log(id, "Lighting control command received but device is not a lighting");
            return;
        }
        if (light_power == 0) {
            light_power = lighting_data->lightPower;
        }
        lighting_data->command_lightPower = light_power;
        lighting_data->command_duration_minutes = duration;
        lighting_data->is_running_on_command = true;
        response = "340 " + to_string(lighting_data->calculate_power()) + " " + to_string(duration) + " " + to_string(light_power);
        send_message(clientfd, response);
        log(id, "Lighting control command accepted, power: " + to_string(light_power) + " Watts for " + to_string(duration) + " minutes");
    } else {
        // reply OK (generic)
        response = "300";
        send_message(clientfd, response);
        log(id, "Control command received for unknown device type, replied OK");     
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

void handle_connect(ClientInfo* info, stringstream& ss, string pass, string &token) {
    cout << "Received connect request from client " << info->ip_str << endl;
    vector<string> params;
    string response;
    if (!read_from_ss(ss, info->clientfd, params, 2, "3", info->self->id)) {
        return;
    }
    int app_id_parsed = stoi(params[0]);
    string inp_pass = params[1];
    cout << "  app_id=" << app_id_parsed << " password=<redacted>" << endl;
    if (info->app_id == app_id_parsed)
    {
        response = "201";
        send_message(info->clientfd, response);
        log(info->self->id, "Client " + string(info->ip_str) + " attempted to reconnect with same app_id " + to_string(app_id_parsed));
        return;
    }
    if (inp_pass == pass)
    {
        info->app_id = app_id_parsed;
        // generate token now and return in format: 2 200 <token>
        token = generate_token(info->self->name);
        response = "200 " + token;
        send_message(info->clientfd, response);
        log(info->self->id, "Client " + string(info->ip_str) + " connected successfully with app_id " + to_string(app_id_parsed));
    }
    else
    {
        response = "202";
        send_message(info->clientfd, response);
        log(info->self->id, "Client " + string(info->ip_str) + " failed to connect with app_id " + to_string(app_id_parsed));
    }
}
void handle_control(string client_ip, SensorDataStructure* data, int clientfd, stringstream& ss, string id) {
    vector<string> params;
    string response;
    if (!(read_from_ss(ss, clientfd, params, 1, "3", id))) {
        return;
    }
    string action = params[0];
    cout << "Received control request from client " << client_ip << ": ";
    if (action == "POWER_ON" || action == "POWER_OFF" || action == "0" || action == "1") {
        string state = (action == "POWER_ON" || action == "1") ? "ON" : "OFF";
        cout << "power control -> set state to " << state << endl;
        if (data->powered_on == true && state == "ON") {
            response = "311";
            send_message(clientfd, response);
            log(id, "Client " + client_ip + " attempted to power ON an already ON sensor");
            return;
        }
        if (data->powered_on == false && state == "OFF") {
            response = "312";
            send_message(clientfd, response);
            log(id, "Client " + client_ip + " attempted to power OFF an already OFF sensor");
            return;
        } 
        data->powered_on = (state == "ON");
        response = "310 " + to_string(data->calculate_power()) + " " + (data->powered_on ? "1" : "0");
        send_message(clientfd, response);
        log(id, "Client " + client_ip + " set power state to " + state);
    } else if (action == "TIMER" || action == "3") {
        if (!read_from_ss(ss, clientfd, params, 2, "3", id)) {
            return;
        }
        string state = params[0];
        int minutes = stoi(params[1]);

        // Kiểm tra dữ liệu hợp lệ
        if (state != "0" && state != "1") {
            response = "3";
            send_message(clientfd, response);
            log(id, "Client " + client_ip + " sent invalid timer state: " + state);
            return;
        }
        if (minutes < 0) {
            response = "3";
            send_message(clientfd, response);
            log(id, "Client " + client_ip + " sent invalid timer minutes: " + to_string(minutes));
            return;
        }
        // Nếu current_time nhỏ hơn thời gian đã đặt thì báo đã đặt hẹn giờ
        time_t current_time = time(nullptr);
        if (data->timer_time > current_time && data->timer_set) {
            response = "351";
            send_message(clientfd, response);
            log(id, "Client " + client_ip + " attempted to set a timer while another timer is already set");
            return;
        }
        // Nếu trạng thái hẹn giờ trùng với trạng thái hiện tại thì báo lỗi
        if (data->powered_on == true && state == "1") {
            response = "352";
            send_message(clientfd, response);
            log(id, "Client " + client_ip + " attempted to set timer to ON while sensor is already ON");
            return;
        }
        if (data->powered_on == false && state == "0") {
            response = "353";
            send_message(clientfd, response);
            log(id, "Client " + client_ip + " attempted to set timer to OFF while sensor is already OFF");
            return;
        }
        
        // Tính thời điểm thực hiện hành động
        time_t action_time = current_time + minutes * 60;
        data->timer_set_to = (state == "1");
        data->timer_time = action_time % 86400 + 7 * 3600; // adjust to GMT+7
        data->timer_set = true;
        cout << "timer control -> set to " << (data->timer_set_to ? "ON" : "OFF") << " in " << minutes << " minutes" << endl;
        response = "350 " + to_string(data->calculate_power()) + " " + to_string(minutes);
        send_message(clientfd, response);
        log(id, "Client " + client_ip + " set timer to " + (data->timer_set_to ? "ON" : "OFF") + " in " + to_string(minutes) + " minutes");
    } else if (action == "CANCEL" || action == "4") {
        if (!data->is_running_on_command) {
            response = "361";
            send_message(clientfd, response);
            cout << "cancel control request failed: no ongoing command" << endl;
            log(id, "Client " + client_ip + " attempted to cancel control with no ongoing command");
            return;
        }
        cout << "cancel control request" << endl;
        response = "360";
        send_message(clientfd, response);
        log(id, "Client " + client_ip + " cancelled the ongoing control command");
        data->is_running_on_command = false;
    } else {
        handle_control_commands(clientfd, ss, data, id);
    }
}

bool update_sensor_configuration(SensorDataStructure* data, int param_index, const string& param_value) {
    cout << "Updating sensor configuration for " << data->type << " with param index: " << param_index << " and value: " << param_value << endl;
    long temp_int_param;
    float temp_float_param;
    if (param_index < 1)
        return false;
    if (data->type == "sprinkler") {
        SprinklerDataStructure* sprinkler_data = dynamic_cast<SprinklerDataStructure*>(data);
        if (sprinkler_data == nullptr) {
            return false;
        }
        switch (param_index) {
            case 1:
                temp_int_param = stoi(param_value);
                if (temp_int_param <= 0) {
                    return false;
                }
                sprinkler_data->t = temp_int_param;
                break;
            case 2:
                temp_int_param = stoi(param_value);
                if (temp_int_param <= 0) {
                    return false;
                }
                sprinkler_data->watering_start_time = temp_int_param % 86400; // ensure it's within a day
                break;
            case 3:
                temp_int_param = stoi(param_value);
                if (temp_int_param <= 0) {
                    return false;
                }
                sprinkler_data->watering_end_time = temp_int_param % 86400; // ensure it's within a day
                break;
            case 4:
                temp_int_param = stoi(param_value);
                if (temp_int_param <= 0) {
                    return false;
                }
                sprinkler_data->volume_per_minute = temp_int_param;
                break;
            case 5:
                temp_float_param = stof(param_value);
                if (temp_float_param < 0 || temp_float_param > 100) {
                    return false;
                }
                sprinkler_data->min_humidity = temp_float_param;
                break;
            case 6:
                temp_float_param = stof(param_value);
                if (temp_float_param < 0 || temp_float_param > 100) {
                    return false;
                }
                sprinkler_data->max_humidity = temp_float_param;
                break;
            default:
                return false;
        }
        sprinkler_data->save_config();
        return true;
    } else if (data->type == "fertilizer") {
        FertilizerDataStructure* fertilizer_data = dynamic_cast<FertilizerDataStructure*>(data);
        if (fertilizer_data == nullptr) {
            return false;
        }
        switch (param_index) {
            case 1:
                temp_int_param = stoi(param_value);
                if (temp_int_param <= 0) {
                    return false;
                }
                fertilizer_data->t = temp_int_param;
                break;
            case 2:
                temp_int_param = stoi(param_value);
                if (temp_int_param <= 0) {
                    return false;
                }
                fertilizer_data->fertilizing_start_time = temp_int_param % 86400;
                break;
            case 3:
                temp_int_param = stoi(param_value);
                if (temp_int_param <= 0) {
                    return false;
                }
                fertilizer_data->fertilizing_end_time = temp_int_param % 86400;
                break;
            case 4:
                temp_float_param = stof(param_value);
                if (temp_float_param < 0) {
                    return false;
                }
                fertilizer_data->fertilizer_amount_nitrogen_per_liter = temp_float_param;
                break;
            case 5:
                temp_float_param = stof(param_value);
                if (temp_float_param < 0) {
                    return false;
                }
                fertilizer_data->min_nitrogen_concentration = temp_float_param;
                break;
            case 6:
                temp_float_param = stof(param_value);
                if (temp_float_param < 0) {
                    return false;
                }
                fertilizer_data->fertilizer_amount_phosphorus_per_liter = temp_float_param;
                break;
            case 7:
                temp_float_param = stof(param_value);
                if (temp_float_param < 0) {
                    return false;
                }
                fertilizer_data->min_phosphorus_concentration = temp_float_param;
                break;
            case 8:
                temp_float_param = stof(param_value);
                if (temp_float_param < 0) {
                    return false;
                }
                fertilizer_data->fertilizer_amount_potassium_per_liter = temp_float_param;
                break;
            case 9:
                temp_float_param = stof(param_value);
                if (temp_float_param < 0) {
                    return false;
                }
                fertilizer_data->min_potassium_concentration = temp_float_param;
                break;
            case 10:
                temp_int_param = stoi(param_value);
                if (temp_int_param <= 0) {
                    return false;
                }
                fertilizer_data->volume_per_minute = temp_int_param;
                break;
            default:
                return false;
        }
        fertilizer_data->save_config();
        return true;
    } else if (data->type == "lighting") {
        LightingDataStructure* lighting_data = dynamic_cast<LightingDataStructure*>(data);
        if (lighting_data == nullptr) {
            return false;
        }
        switch (param_index) {
            case 1:
                temp_int_param = stoi(param_value);
                if (temp_int_param <= 0) {
                    return false;
                }
                lighting_data->t = temp_int_param;
                break;
            case 2:
                temp_int_param = stoi(param_value);;
                if (temp_int_param <= 0) {
                    return false;
                }
                lighting_data->lighting_start_time = temp_int_param % 86400; // ensure it's within a day
                break;
            case 3:
                temp_int_param = stoi(param_value);
                if (temp_int_param <= 0) {
                    return false;
                }
                lighting_data->lighting_end_time = temp_int_param % 86400; // ensure it's within a day
                break;
            case 4:
                temp_int_param = stoi(param_value);;
                if (temp_int_param <= 0) {
                    return false;
                }
                lighting_data->lightPower = temp_int_param;
                break;
            default:
                return false;
        }
        lighting_data->save_config();
        return true;
    }
    return false;
}