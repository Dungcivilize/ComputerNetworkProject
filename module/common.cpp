#include <iostream>
#include "common.hpp"
#include <sstream>
#include "../dependencies/framework.hpp"
#include "../dependencies/streamtransmission.hpp"
#include "../dependencies/utils.hpp"
#define BUFFER_SIZE 32787

using namespace std;
int check_args(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return -1;
    }
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        printf("Invalid port. Port must be between 0 and 65535\n");
        return -1;
    }
    return port;
}

void call_api(int sockfd, string request) {
    std::string response;
    if (send_message(sockfd, request) <= 0) {
        cout << "Failed to send message to device." << endl;
        return;
    }
    if (recv_message(sockfd, response) <= 0) {
        cout << "Failed to receive message from device." << endl;
        return;
    }
    hanlde_response(response);
}

string hanlde_response(string response) {
    string action, status_code, power_data, data;
    stringstream ss(response);
    ss >> action >> status_code >> power_data;
    getline(ss, data);
    cout << get_status_message(stoi(status_code)) << endl;
    if (action == "POWER_ON" || action == "POWER_OFF") {
        cout << "Power state changed to " << (action == "POWER_ON" ? "ON" : "OFF") << endl;
        cout << "Power consumption: " << power_data << " Watts" << endl;
    }
    if (stoi(status_code) % 100 != 0) {
        return;
    }
    if (action == "WATER_NOW") {
        cout << "Watering amount: " << data << " Liters" << endl;
        cout << "Power consumption: " << power_data << " Watts" << endl;
    }
    else if (action == "FERTILIZE_NOW") {
        cout << "Fertilizing amount: " << endl;
        stringstream data_ss(data);
        string item;
        while (getline(data_ss, item, ' ')) {
            if (!item.empty()) {
                cout << "-  " << item << endl;
            }
        }
        cout << "Power consumption: " << power_data << " Watts" << endl;
    }
    else if (action == "LIGHT_NOW") {
        cout << "Lighting duration and power set." << endl;
        int duration = 0, power = 0;
        stringstream data_ss(data);
        data_ss >> duration >> power;
        cout << "Duration: " << duration << " minutes" << endl;
        cout << "Power: " << power << " Watts" << endl;
        cout << "Power consumption: " << power_data << " Watts" << endl;
    }
    else if (action == "TIMER") {
        // Tính thời điểm thực hiện hành động
        time_t current_time = time(nullptr);
        int minutes = stoi(data);
        time_t action_time = current_time + minutes * 60;
        char time_str[100];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&action_time));
        cout << "Action scheduled at: " << time_str << endl;
        cout << "Power consumption: (expected) " << power_data << " Watts" << endl;
    }
    else if (action == "CANCEL_CONTROL") {
        cout << "Scheduled action cancelled." << endl;
    }
    else if (action == "QUERY") {
        cout << "Device Data:" << endl;
        stringstream data_ss(data);
        string item;
        while (getline(data_ss, item, ';')) {
            if (!item.empty()) {
                cout << "- " << item << endl;
            }
        }
    }
    else {
        cout << "Unhandled action: " << action << endl;
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
            return "Authentication failed. Invalid token or app ID.";
        case 3: 
            return "Invalid command format.";
        case 4:
            return "Unknown command.";
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
            return "Device's power state already on.";
        case 311:
            return "Device's power state already off.";
        case 320:
            return "Humidity reaches its maximum threshold. Watering not started.";
        case 321:
            return "Insufficient water in the tank. Watering not started.";
        case 340:
            return "Device already has a timer set.";
        case 341:
            return "Device already on";
        case 342:
            return "Device already off";
        case 350:
            return "No action to cancel.";
        case 500:
            return "Get device status successful.";
        case 501:
            return "Device not found.";
        default:
            return "Unknown status code.";
    }
}