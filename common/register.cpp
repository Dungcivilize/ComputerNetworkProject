#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <cstdlib>
#include "register.hpp"
#include <climits>
int check() {
    std::ifstream file("app_id.data");
    if (!file.is_open()) {
        return -1;
    }
    int app_id;
    file >> app_id;
    file.close();
    return app_id;
}

int register_app() {
    int app_id = check();
    if (app_id != -1) {
        return app_id;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, INT_MAX);
    app_id = dist(gen);
    
    std::ofstream file("app_id.data");
    if (file.is_open()) {
        file << app_id << std::endl;
        file.close();
    } else {
        std::cerr << "Error: Cannot open app_id.data for writing" << std::endl;
    }
    return app_id;
}

void cancel_register() {
    if (std::remove("app_id.data") != 0) {
        std::cerr << "Error: Cannot delete app_id.data" << std::endl;
    }
}
