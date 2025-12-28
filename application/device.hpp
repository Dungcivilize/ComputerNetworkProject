#pragma once

#include "framework.hpp"

struct DeviceInfo
{
    sockaddr_in addr;
    string id;
    string type;

    DeviceInfo(sockaddr_in addr, string id, string type) : addr(addr), id(id), type(type) {} 
};

/* Device struct for storing connection socket and device's info. */
struct Device
{
    int fd;
    string id;
    string type;
    string token;
    vector<int> params;

    // Main constructor
    Device(int fd, string id, string type, string token) : fd(fd), id(id), type(type), token(token) {}

    // Implement rules to avoid duplicate file descriptor ownership
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    Device(Device&& other) noexcept : fd(other.fd), id(std::move(other.id)),
        type(std::move(other.type)), token(std::move(other.token)), params(std::move(other.params)) 
    {
        other.fd = -1;
    }
    Device& operator=(Device&& other) noexcept 
    {
        if (this != &other) 
        {
            if (fd >= 0) close(fd);
            fd = other.fd;
            other.fd = -1;
            id = std::move(other.id);
            type = std::move(other.type);
            token = std::move(other.token);
            params = std::move(other.params);
        }
        return *this;
    }

    // Close socket upon destroying object
    ~Device() { if (fd >= 0) close(fd); }

    string display()
    {
        string params;
        if (type == "sensor")
            params = "T: " + to_string(this->params[1]); 
        else if (type == "light")
            params = "P: " + to_string(this->params[1]);
        else if (type == "sprinkler")
        {
            params = "Hmin: " + to_string(this->params[1]) +
            "\nHmax: " + to_string(this->params[2]);
        }
        else if (type == "fertilizer")
        {
            params = "C: " + to_string(this->params[1]) +
            "\nV: " + to_string(this->params[2]) +
            "\nNmin: " + to_string(this->params[3]) +
            "\nPmin: " + to_string(this->params[4]) +
            "\nKmin: " + to_string(this->params[5]);
        }
        return "ID: " + this->id +
        "\nType: " + this->type + 
        "\nPower: " + (this->params[0] ? "ON" : "OFF") +
        "\n" + params;
    }

    void set_params(vector<int> params)
    {
        this->params = params;
    }
};