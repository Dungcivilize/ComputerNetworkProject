#ifndef SENSORSERVER_SENSOR_HPP
#define SENSORSERVER_SENSOR_HPP

#include "../dependencies/index.hpp"

#include <random>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <errno.h>
#include "sensorDataStructure.hpp"

// forward declare to allow ClientInfo to hold a Sensor* before Sensor is defined
class Sensor;

class ClientInfo {
    public:
        Sensor* self;
        int clientfd;
        int app_id;
        ClientInfo(Sensor* self, int clientfd, int app_id) : self(self), clientfd(clientfd), app_id(app_id) {}   
};
#include "sensor_module_fwd.hpp"


class Sensor
{
public:
    string id;
    string sensor_type;
    string name;
    string pass;

    SensorDataStructure* data;

    // per-client sockets are handled in threads; no global userfd/connected
    Identity* tcpid = nullptr;

    Sensor(uint16_t port, const string& sensor_id, string sensor_type, const string& sensor_name, const string& sensor_pass)
    {
        id = sensor_id;
        name = sensor_name;
        pass = sensor_pass;
        tcpid = create_identity(port, SOCK_STREAM, SOL_SOCKET, SO_REUSEADDR, INADDR_ANY);
        this->sensor_type = sensor_type;
        if (sensor_type == "SPRINKLER") {
            data = new SprinklerDataStructure(id);
        } else if (sensor_type == "FERTILIZER") {
            data = new FertilizerDataStructure(id);
        } else if (sensor_type == "LIGHTING") {
            data = new LightingDataStructure(id);
        } else {
            data = new SensorDataStructure();
        }
    }

    // Convenience overload used by tests: default sensor_type to "SENSOR"
    Sensor(uint16_t port, const string& sensor_id, const string& sensor_name, const string& sensor_pass)
        : Sensor(port, sensor_id, string("SENSOR"), sensor_name, sensor_pass) {}

    ~Sensor()
    {
        if (tcpid) delete tcpid;
    }

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
        // determine peer IP for logging
        struct sockaddr_in peer_addr;
        socklen_t peer_len = sizeof(peer_addr);
        char peer_ip[INET_ADDRSTRLEN] = "unknown";
        if (getpeername(clientfd, (struct sockaddr*)&peer_addr, &peer_len) == 0) {
            inet_ntop(AF_INET, &peer_addr.sin_addr, peer_ip, sizeof(peer_ip));
        }
        string buf, cmd;
        vector<string> params;
        string response;
        string token, inp_token;
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
                continue;
            } else if (cmd == "2") {
                handle_connect(peer_ip, info, ss, pass, token);
                continue;
            } 
            if (!requires_authentication(token, ss, clientfd)) {
                continue;
            } 
            if (cmd == "3") {
                handle_control(peer_ip, data, clientfd, ss);
            } else if (cmd == "4") {
                if (!read_from_ss(ss, clientfd, params, 2)) {
                    continue;
                }
                string old_pass = params[0];
                string new_pass = params[1];
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
            } else if (cmd == "5") {
                // Query data
                cout << "Received query request from client " << peer_ip << endl;
                string data_str = data->toString();
                response = "500 " + to_string(data->is_running_on_command) + " " + data_str;
                send_message(clientfd, response);
            } else if (cmd == "6") {
            
            } else if (cmd == "7") {
                cout << "Client " << peer_ip << " disconnected." << endl;
                info->app_id = -1;
                send_message(clientfd, "700");
            } else if (cmd == "8") {
                cout << "Cancel Timer request from client " << peer_ip << endl;
                if (!data->timer_set) {
                    response = "801";
                    send_message(clientfd, response);
                    continue;
                }
                data->timer_set = false;
                response = "800";
                send_message(clientfd, response);
            } else {
                response = "4";
                send_message(clientfd, response);
            }
        }
        close(info->clientfd);
        delete info;
    }

    void check_sensor()
    {
        time_t start_time = time(nullptr);
        while (1) {
            sleep(60); // check every minute
            time_t end_time = time(nullptr);
            if (data->powered_on == false) {
                start_time = end_time;
                continue;
            }
            if (end_time - start_time >= data->t * 60) {
                data->run(start_time, end_time);
                start_time = end_time;
            }
        }

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
        // Start sensor check thread
        pthread_t check_thread;
        if (pthread_create(&check_thread, NULL, [](void* arg) -> void*
            {
                Sensor* self = (Sensor*)arg;
                self->check_sensor();
                return NULL;
            }, this) != 0)
        {
            perror("pthread_create check_sensor");
            close(tcpid->sockfd);
            return;
        }
        pthread_detach(check_thread);
        // Start accepting clients
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
            ClientInfo* info = new ClientInfo(this, clientfd, -1);  
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


#include "sensor_module.hpp"

#endif // SENSORSERVER_SENSOR_HPP
