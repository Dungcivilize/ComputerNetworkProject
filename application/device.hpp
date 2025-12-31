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
    int state;
    vector<int> params;
    vector<string> schedule;

    // Main constructor
    Device(int fd, string id, string type, string token) : fd(fd), id(id), type(type), token(token) {}

    // Implement rules to avoid duplicate file descriptor ownership
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    Device(Device&& other) noexcept : fd(other.fd), id(std::move(other.id)),
        type(std::move(other.type)), token(std::move(other.token)), state(std::move(other.state)), params(std::move(other.params)) , schedule(std::move(other.schedule))
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
            state = std::move(other.state);
            params = std::move(other.params);
            schedule = std::move(other.schedule);
        }
        return *this;
    }

    // Close socket upon destroying object
    ~Device() { if (fd >= 0) close(fd); }

    string display()
    {
        string params = display_param();
        return "ID: " + this->id +
        "\nType: " + this->type + 
        "\nState: " + (this->state ? "ON" : "OFF") +
        "\n" + params;
    }

    string display_schedule()
    {
        if (schedule.empty()) return "NaN";
        string resp;
        for (auto& timestamp : schedule) resp += timestamp + " ";
        return "Schedule: " + resp; 
    }

    string display_param()
    {
        string params;
        if (type == "sensor")
            params = "T: " + to_string(this->params[0]); 
        else if (type == "light")
        {
            params = "P: " + to_string(this->params[0]) +
            "\nS: " + to_string(this->params[1]);
        }
        else if (type == "sprinkler")
        {
            params = "Hmin: " + to_string(this->params[0]) +
            "\nHmax: " + to_string(this->params[1]) +
            "\nV: " + to_string(this->params[2]);
        }
        else if (type == "fertilizer")
        {
            params = "C: " + to_string(this->params[0]) +
            "\nV: " + to_string(this->params[1]) +
            "\nNmin: " + to_string(this->params[2]) +
            "\nPmin: " + to_string(this->params[3]) +
            "\nKmin: " + to_string(this->params[4]);
        }
        return params;
    }

    string display_state()
    {
        return string("State: ") + (this->state ? "ON" : "OFF");
    }
};