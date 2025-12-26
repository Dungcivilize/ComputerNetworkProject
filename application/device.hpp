#pragma once

#include "framework.hpp"

struct DeviceInfo
{
    sockaddr_in addr;
    string id;
    string name;
    string type;

    DeviceInfo(sockaddr_in addr, string id, string name, string type) : addr(addr), id(id), name(name), type(type) {} 
};

/* Device struct for storing connection socket and device's info. */
struct Device
{
    int fd;
    string id;
    string name;
    string type;
    string token;
    vector<int> params;

    // Main constructor
    Device(int fd, string id, string name, string type, string token, vector<int> params) : fd(fd), id(id), name(name), type(type), token(token), params(params) {}

    // Implement rules to avoid duplicate file descriptor ownership
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    Device(Device&& other) noexcept : fd(other.fd), id(std::move(other.id)),
        name(std::move(other.name)), type(std::move(other.type)), token(std::move(other.token)), params(std::move(other.params)) 
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
            name = std::move(other.name);
            type = std::move(other.type);
            token = std::move(other.token);
            params = std::move(other.params);
        }
        return *this;
    }

    // Close socket upon destroying object
    ~Device() { if (fd >= 0) close(fd); }
};