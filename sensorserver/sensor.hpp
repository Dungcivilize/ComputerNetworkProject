#ifndef SENSORSERVER_SENSOR_HPP
#define SENSORSERVER_SENSOR_HPP

#include "../dependencies/index.hpp"

#include <random>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <errno.h>

class Sensor
{
public:
    string id;
    string sensor_type;
    string name;
    string pass;
    float T = 0;

    // per-client sockets are handled in threads; no global userfd/connected
    Identity* tcpid = nullptr;

    Sensor(uint16_t port, const string& sensor_id, const string& sensor_name, const string& sensor_pass)
    {
        id = sensor_id;
        name = sensor_name;
        pass = sensor_pass;
        tcpid = create_identity(port, SOCK_STREAM, SOL_SOCKET, SO_REUSEADDR, INADDR_ANY);
        sensor_type = "SENSOR";
    }

    // Generate a token derived from sensor name and random entropy
    string generate_token(size_t bytes = 16)
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

    ~Sensor()
    {
        if (tcpid) delete tcpid;
    }

    struct ClientInfo {
        Sensor* self;
        int clientfd;
        int app_id;
    };

    static void* client_thread_start(void* arg)
    {
        ClientInfo* info = (ClientInfo*)arg;
        Sensor* self = info->self;
        self->client_session(info);
        return NULL;
    }

    void client_session(ClientInfo* info)
    {
        int clientfd = info->clientfd;
        string buf, cmd;
        string response;
        string token, inp_token;
        bool authenticated = false;
        while (1)
        {
            ssize_t m = recv_message(clientfd, buf);
            if (m <= 0) break; // disconnected or error

            stringstream ss(buf);
            ss >> cmd;
            if (cmd == "1") {
                response = "100 " + id + " " + name + " " + sensor_type;
                send_message(clientfd, response);
            } else if (cmd == "2") {
                int app_id_parsed;
                string inp_pass;
                if (!(ss >> app_id_parsed >> inp_pass))
                {
                    response = "3";
                    send_message(clientfd, response);
                    continue;
                }
                if (info->app_id == app_id_parsed)
                {
                    response = "201";
                    send_message(clientfd, response);
                    continue;
                }
                if (inp_pass == pass)
                {
                    authenticated = true;
                    info->app_id = app_id_parsed;
                    // generate token now and return in format: 2 200 <token>
                    token = generate_token();   
                    response = "200 " + token;
                    send_message(clientfd, response);
                }
                else
                {
                    response = "202";
                    send_message(clientfd, response);
                }
            } else if (cmd == "4") {
                string old_pass, new_pass;
                if (!(ss >> inp_token >> old_pass >> new_pass))
                {
                    response = "3";
                    send_message(clientfd, response);
                    continue;
                }
                if (inp_token != token)
                {
                    response = "2";
                    send_message(clientfd, response);
                    continue;
                }
                if (old_pass != pass)
                {
                    response = "401";
                    send_message(clientfd, response);
                }
                else
                {
                    if (new_pass == pass)
                    {
                        response = "402";
                        send_message(clientfd, response);
                        continue;
                    }
                    pass = new_pass;
                    response = "400";
                    send_message(clientfd, response);
                }
            } else if (cmd == "CONFIG") {
                if (!authenticated)
                {
                    string msg = to_string(ERROR_UNKNOWN_COMMAND) + " Not connected";
                    send_message(clientfd, msg);
                    continue;
                }
                string param;
                float value;
                ss >> param >> value;
                if (param != "T")
                {
                    string msg = to_string(ERROR_INVALID_PARAM) + " No such param for sensor";
                    send_message(clientfd, msg);
                }
                else
                {
                    T = value;
                    cout << "T has been set to: " << T << endl;
                    string msg = to_string(SUCCESS_PARAM_CHANGE) + " Sensor T value configured";
                    send_message(clientfd, msg);
                }
            } else {
                response = "4";
                send_message(clientfd, response);
            }
        }
        close(info->clientfd);
        delete info;
    }

    void run()
    {
        string buf, cmd;

        // Start listening once like a normal TCP server
        if (listen(tcpid->sockfd, SOMAXCONN) < 0)
        {
            perror("listen");
            close(tcpid->sockfd);
            return;
        }

        while (1)
        {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int clientfd = accept(tcpid->sockfd, (struct sockaddr*)&client_addr, &client_len);
            if (clientfd < 0)
            {
                perror("accept");
                continue;
            }

            // spawn detached thread to handle client
            ClientInfo* info = new ClientInfo{this, clientfd, 0};
            pthread_t tid;
            if (pthread_create(&tid, NULL, Sensor::client_thread_start, info) != 0)
            {
                perror("pthread_create client");
                close(clientfd);
                delete info;
                continue;
            }
            pthread_detach(tid);
        }
    }
};

#endif // SENSORSERVER_SENSOR_HPP
