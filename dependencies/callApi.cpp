#include "utils.hpp"
#include "../dependencies/index.hpp"
#include "../common/dataStructure.hpp"
#include "../common/common.hpp"
#include <iostream>
#include "logger.hpp"

using namespace std;    

void handle_response(std::string response, void* output);
std::string get_status_message(int status_code);

void call_api(int sockfd, string request, void* output) {
    std::string response;
    log("Sending request: \"" + request + "\" to socket fd: " + std::to_string(sockfd));
    if (send_message(sockfd, request) <= 0) {
        cout << "Failed to send message to device." << endl;
        log("Error sending request \"" + request + "\" to socket fd: " + std::to_string(sockfd));
        return;
    }
    if (recv_message(sockfd, response) <= 0) {
        cout << "Failed to receive message from device." << endl;
        log("Error receiving message of request \"" + request + "\" from socket fd: " + std::to_string(sockfd));
        return;
    }
    log("Received response: " + response);
    handle_response(response, output);
}

void handle_response(std::string response, void* output) {
    string status_code, power_data, data;
    stringstream ss(response);
    ss >> status_code;
    if (status_code == "351" && output != nullptr) {
        // Special case for 351 to handle user prompt
        char choice;
        cout << "A timer is already set for this device. Cancel this timer? (y/n): ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        *((char*)output) = choice;
        return;
    }
    cout << get_status_message(stoi(status_code)) << endl;
    if (stoi(status_code) % 10 != 0) {
        return;
    }
    // Declare variables outside the switch to avoid bypassing initialization
    stringstream data_ss;
    string item;

    switch (stoi(status_code)) {
        case 200: {
            ss >> data;
            cout << "Device token: " << data << endl;
            if (output) {
                std::string* out_str = static_cast<std::string*>(output);
                *out_str = data;
            }
            break;
        }
        case 300:
            // Nếu là 300 thì báo rằng không có trạng thái trả về do chỉ là xác nhận thành công
            cout << "No additional data for control confirmation because target device only confirms success." << endl;
            break;
        case 310:
            ss >> power_data >> data;
            cout << "Power state changed to " << (data == "1" ? "ON" : "OFF") << endl;
            cout << "Power: " << power_data << " Watts" << endl;
            break;
        case 320:
            ss >> power_data >> data;
            cout << "Watering amount: " << data << " Liters" << endl;
            cout << "Power: " << power_data << " Watts" << endl;
            break;
        case 330:               
            ss >> power_data;
            getline(ss, data);
            cout << "Fertilizing amount: " << endl;
            data_ss.clear();
            data_ss.str(data);
            float cn, cp, ck, volume;
            data_ss >> volume >> cn >> cp >> ck;
            cout << "  Water: " << volume << " Liters" << endl;
            cout << "  Nitrogen: " << cn << " ppm" << endl;
            cout << "  Phosphorus: " << cp << " ppm" << endl;
            cout << "  Potassium: " << ck << " ppm" << endl;
            cout << "Power consumption: " << power_data << " Watts" << endl;
            break;
        case 340:    {
            int duration = 0, power = 0;
            ss >> power_data >> duration >> power;
            cout << "Lighting duration and power set." << endl;
            cout << "Duration: " << duration << " minutes" << endl;
            cout << "Power: " << power << " Watts" << endl;
            cout << "Power consumption: " << power_data << " Watts" << endl;
            break;
        }
        case 350: {
            ss >> power_data >> data;
            // Tính thời điểm thực hiện hành động
            time_t current_time = time(nullptr);
            int minutes = stoi(data);
            time_t action_time = current_time + minutes * 60;
            char time_str[100];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&action_time));
            cout << "Action scheduled at: " << time_str << endl;
            cout << "Power consumption: (expected) " << power_data << " Watts" << endl;
            break;
        }
        case 360: {
            ss >> data;
            cout << "Cancelled action: " << data << endl;
            break;
        }
        case 500: {
            cout << "Device Data:" << endl;
            // Chuyển output thành enum 
            DeviceType* device_type = static_cast<DeviceType*>(output);
            bool is_running_on_command;
            ss >> is_running_on_command;
            vector<string> params;
            switch (*device_type) {
                case SPRINKLER: {
                    for (int i = 0; i < 10; ++i) {
                        ss >> item;
                        params.push_back(item);
                    }
                    int volume_per_minute = stoi(params[0]);
                    float humidity = stof(params[1]);
                    float min_humidity = stof(params[2]);
                    float max_humidity = stof(params[3]);
                    string watering_start_time = convert_time_t_to_string_only_time(stol(params[4]));
                    string watering_end_time = convert_time_t_to_string_only_time(stol(params[5]));
                    int current_water_amount = stoi(params[6]);
                    int tank_capacity = stoi(params[7]);
                    int t = stoi(params[8]);
                    float total_energy_consumed = stof(params[9]);
                    cout << "  Sprinkler sensor data:" << endl;
                    cout << "  Wunning on command: " << (is_running_on_command ? "Yes" : "No") << endl;
                    cout << "  Water Volume per Minute: " << volume_per_minute << " Liters" << endl;
                    cout << "  Humidity: " << humidity << "%" << endl;
                    cout << "  Min Humidity: " << min_humidity << "%" << endl;
                    cout << "  Max Humidity: " << max_humidity << "%" << endl;
                    cout << "  Watering Start Time: " << watering_start_time << endl;
                    cout << "  Watering End Time: " << watering_end_time << endl;
                    cout << "  Current Water Amount: " << current_water_amount << " Liters" << endl;
                    cout << "  Tank Capacity: " << tank_capacity << " Liters" << endl;
                    cout << "  Sensor refresh time: " << t << " minutes" << endl;
                    cout << "  Total Energy Consumed: " << total_energy_consumed << " kWh" << endl;
                    break;
                }
                case FERTILIZER: {
                    for (int i = 0; i < 13; ++i) {
                        ss >> item;
                        params.push_back(item);
                    }
                    float fertilizer_amount_nitrogen_per_liter = stof(params[0]);
                    float nitrogen_concentration = stof(params[1]);
                    float min_nitrogen_concentration = stof(params[2]);
                    float fertilizer_amount_phosphorus_per_liter = stof(params[3]);
                    float phosphorus_concentration = stof(params[4]);
                    float min_phosphorus_concentration = stof(params[5]);
                    float fertilizer_amount_potassium_per_liter = stof(params[6]);
                    float potassium_concentration = stof(params[7]);
                    float min_potassium_concentration = stof(params[8]);
                    string fertilizing_start_time = convert_time_t_to_string_only_time(stol(params[9]));
                    string fertilizing_end_time = convert_time_t_to_string_only_time(stol(params[10]));
                    int t = stoi(params[11]);
                    float total_energy_consumed = stof(params[12]);
                    cout << "  Fertilizer sensor data:" << endl;
                    cout << "  Fertilizing on command: " << (is_running_on_command ? "Yes" : "No") << endl;
                    cout << "  Fertilizer Amount per Liter - Nitrogen: " << fertilizer_amount_nitrogen_per_liter << " mg/L" << endl;
                    cout << "  Nitrogen Concentration: " << nitrogen_concentration << " ppm" << endl;
                    cout << "  Min Nitrogen Concentration: " << min_nitrogen_concentration << " ppm" << endl;
                    cout << "  Fertilizer Amount per Liter - Phosphorus: " << fertilizer_amount_phosphorus_per_liter << " mg/L" << endl;
                    cout << "  Phosphorus Concentration: " << phosphorus_concentration << " ppm" << endl;
                    cout << "  Min Phosphorus Concentration: " << min_phosphorus_concentration << " ppm" << endl;
                    cout << "  Fertilizer Amount per Liter - Potassium: " << fertilizer_amount_potassium_per_liter << " mg/L" << endl;
                    cout << "  Potassium Concentration: " << potassium_concentration << " ppm" << endl;
                    cout << "  Min Potassium Concentration: " << min_potassium_concentration << " ppm" << endl;
                    cout << "  Fertilizing Start Time: " << fertilizing_start_time << endl;
                    cout << "  Fertilizing End Time: " << fertilizing_end_time << endl;
                    cout << "  Sensor refresh time: " << t << " minutes" << endl;
                    cout << "  Total Energy Consumed: " << total_energy_consumed << " kWh" << endl;
                    break;
                }
                case LIGHTING: {
                    for (int i = 0; i < 5; ++i) {
                        ss >> item;
                        params.push_back(item);
                    }
                    int lightPower = stoi(params[0]);
                    string lighting_start_time = convert_time_t_to_string_only_time(stol(params[1]));
                    string lighting_end_time = convert_time_t_to_string_only_time(stol(params[2]));
                    int t = stoi(params[3]);
                    float total_energy_consumed = stof(params[4]);
                    cout << "  Lighting sensor data:" << endl;
                    cout << "  Lighting on command: " << (is_running_on_command ? "Yes" : "No") << endl;
                    cout << "  Light Power: " << lightPower << " Watts" << endl;
                    cout << "  Lighting Start Time: " << lighting_start_time << endl;
                    cout << "  Lighting End Time: " << lighting_end_time << endl;
                    cout << "  Sensor refresh time: " << t << " minutes" << endl;
                    cout << "  Total Energy Consumed: " << total_energy_consumed << " kWh" << endl;
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

string get_status_message(int status_code) {
    switch (status_code) {
        case 1:
            return "Invalid parameters provided.";
        case 2:
            return "Authentication failed. Invalid token.";
        case 3: 
            return "Invalid command format.";
        case 4:
            return "Unknown command.";
        case 200:
            return "Connect to device successful.";
        case 201:
            return "Device is already connected.";
        case 202:
            return "Wrong password.";
        case 300:
            return "Device control successful.";
        case 301:
            return "The device is off. Cannot perform the requested operation.";
        case 302:
            return "Device is busy performing another operation.";
        case 303:
            return "The device does not have this function.";
        case 304:
            return "Device not found.";
        case 310:
            return "Device power state changed successfully.";
        case 311:
            return "Device's power state already on.";
        case 312:
            return "Device's power state already off.";
        case 320:
            return "Watering started successfully.";
        case 321:
            return "Humidity reaches its maximum threshold. Watering not started.";
        case 322:
            return "Insufficient water in the tank. Watering not started.";
        case 330:
            return "Fertilizing started successfully.";
        case 331:
            return "Insufficient water in the tank. Fertilizing not started.";
        case 340:
            return "Lighting started successfully.";
        case 350:
            return "Timer set successfully.";
        case 352:
            return "Device already on";
        case 353:
            return "Device already off";
        case 354:
            return "No action to cancel.";
        case 360:
            return "Scheduled action cancelled successfully.";
        case 361:
            return "No scheduled action to cancel.";
        case 400:
            return "Change password successful.";
        case 401:
            return "Current password is incorrect.";
        case 402:
            return "New password haven't changed (same as current password).";
        case 500:
            return "Get device status successful.";
        case 501:
            return "Device not found.";
        case 600:
            return "Configuration updated successfully.";
        case 601:
            return "Invalid configuration parameter.";
        case 700:
            return "Device disconnected successfully.";
        case 800:
            return "Timer cancelled successfully.";
        case 801:
            return "No timer to cancel.";
        default:
            return "";
    }
}