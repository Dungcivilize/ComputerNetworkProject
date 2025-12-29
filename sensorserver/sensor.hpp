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

    void handle_control_commands(int clientfd, const string& token, const string& peer_ip, stringstream& ss, string& response)
    {
        // control commands — print in English and optionally act
                // expected format: 3 <token> <action> [params...]
                string in_token, action;
                ss >> in_token >> action;
                if (in_token != token)
                {
                    response = "2";
                    send_message(clientfd, response);
                    return;
                }
                cout << "Received control request from client " << peer_ip << ": ";
                if (action == "POWER_ON" || action == "POWER_OFF" || action == "0" || action == "1") {
                    string state = (action == "POWER_ON" || action == "0") ? "ON" : "OFF";
                    cout << "power control -> set state to " << state << endl;
                } else if (action == "TIMER") {
                    string state; int minutes;
                    ss >> state >> minutes;
                    cout << "timer -> state=" << state << " minutes=" << minutes << endl;
                } else if (action == "CANCEL") {
                    cout << "cancel control request" << endl;
                } else {
                    // other possible control forms: numeric params
                    vector<string> rest;
                    string tokenp;
                    while (ss >> tokenp) rest.push_back(tokenp);
                    cout << "action=" << action << " params:";
                    for (auto &r: rest) cout << " " << r;
                    cout << endl;
                }
                // reply OK (generic)
                response = "300";
                send_message(clientfd, response);
    }

    void client_session(ClientInfo* info)
    {
        int clientfd = info->clientfd;
        // determine peer IP for logging
        struct sockaddr_in peer_addr;
        socklen_t peer_len = sizeof(peer_addr);
        char peer_ip[INET_ADDRSTRLEN] = "unknown";
        if (getpeername(clientfd, (struct sockaddr*)&peer_addr, &peer_len) == 0) {
            inet_ntop(AF_INET, &peer_addr.sin_addr, peer_ip, sizeof(peer_ip));
        }

        string buf, cmd;
        string response;
        string token, inp_token;
        bool authenticated = false;

        // helper: generate random sensor stats (3 values)
        auto generate_random_stats = [&]() -> string {
            random_device rd;
            mt19937 gen(rd());
            uniform_real_distribution<> dist(0.0, 100.0);
            double a = dist(gen);
            double b = dist(gen);
            double c = dist(gen);
            std::ostringstream oss;
            oss << fixed << setprecision(2) << a << "," << b << "," << c;
            return oss.str();
        };
        while (1)
        {
            ssize_t m = recv_message(clientfd, buf);
            if (m <= 0) break; // disconnected or error

            stringstream ss(buf);
            ss >> cmd;
            if (cmd == "1") {
                cout << "Received scan request from client " << peer_ip << endl;
                response = "100 " + id + " " + name + " " + sensor_type;
                send_message(clientfd, response);
            } else if (cmd == "2") {
                cout << "Received connect request from client " << peer_ip << endl;
                int app_id_parsed;
                string inp_pass;
                if (!(ss >> app_id_parsed >> inp_pass))
                {
                    response = "3";
                    send_message(clientfd, response);
                    continue;
                }
                cout << "  app_id=" << app_id_parsed << " password=<redacted>" << endl;
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
            } else if (cmd == "3") {
                handle_control_commands(clientfd, token, peer_ip, ss, response);
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
