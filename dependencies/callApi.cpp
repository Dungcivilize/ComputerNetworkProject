#include "utils.hpp"
#include "../dependencies/index.hpp"
#include <iostream>

using namespace std;    

void handle_response(std::string response, void* output);
std::string get_status_message(int status_code);

void call_api(int sockfd, string request, void* output) {
    std::string response;
    if (send_message(sockfd, request) <= 0) {
        cout << "Failed to send message to device." << endl;
        return;
    }
    if (recv_message(sockfd, response) <= 0) {
        cout << "Failed to receive message from device." << endl;
        return;
    }
    cout << "Response from device: " << response << endl;
    handle_response(response, output);
}

void handle_response(std::string response, void* output) {
    string status_code, power_data, data;
    stringstream ss(response);
    ss >> status_code;
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
            cout << "Power consumption: " << power_data << " Watts" << endl;
            break;
        case 320:
            ss >> power_data >> data;
            cout << "Watering amount: " << data << " Liters" << endl;
            cout << "Power consumption: " << power_data << " Watts" << endl;
            break;
        case 330:               
            ss >> power_data;
            getline(ss, data);
            cout << "Fertilizing amount: " << endl;
            data_ss.clear();
            data_ss.str(data);
            while (getline(data_ss, item, '|')) {
                if (!item.empty()) {
                    cout << "-  " << item << endl;
                }
            }
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
        case 500:
            cout << "Device Data:" << endl;
            data_ss.clear();
            data_ss.str(data);
            while (getline(data_ss, item, ';')) {
                if (!item.empty()) {
                    cout << "- " << item << endl;
                }
            }
            break;
        default:
            cout << "Unhandled action response." << endl;
            break;
    }
}

string get_status_message(int status_code) {
    if (status_code >= 3300 && status_code < 3399) {
        int error_code = status_code - 3300;
        // Chuyển về dạng nhị phân để đọc lỗi
        bool error_flags[5] = {false};
        for (int i = 0; i < 5; ++i) {
            error_flags[i] = (error_code >> i) & 1;
        }
        cout << "Fertilizing completed with following issues:" << endl;
        if (error_flags[0]) cout << "- Insufficient phosphorus fertilizer." << endl;
        if (error_flags[1]) cout << "- Insufficient potassium fertilizer." << endl;
        if (error_flags[2]) cout << "- Insufficient nitrogen fertilizer." << endl;
        if (error_flags[3]) cout << "- Insufficient water in the tank." << endl;
        if (error_flags[4]) cout << "- Humidity reaches its maximum threshold." << endl;
    }
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
        case 340:
            return "Lighting started successfully.";
        case 350:
            return "Timer set successfully.";
        case 351:
            return "Device already has a timer set.";
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
        case 500:
            return "Get device status successful.";
        case 501:
            return "Device not found.";
        default:
            return "Unknown status code.";
    }
}