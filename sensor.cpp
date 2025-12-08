#include "dependencies/framework.hpp"
#include "dependencies/identity.hpp"
#include "dependencies/utils.hpp"

const string ID = "sensor_id_0";
const string TYPE = "SENSOR";
string name = "unnamed_sensor";
string pass = "123456789";
float T = 0;

int userfd = -1;
struct sockaddr_in usr_addr;
socklen_t usr_addr_len = sizeof(usr_addr);

int connected = 0;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

vector<double> nutrient_evaluate()
{
    vector<double> stats;
    random_device rd;
    mt19937 rng(rd());
    uniform_real_distribution<> dist(0, 1);
    for (int count = 0; count < 3; count++)
        stats.push_back(dist(rng));
    return stats;
}

void* broadcast_listening(void* arg)
{
    Identity* udpid = (Identity*)arg;

    while (1)
    {
        pthread_mutex_lock(&mtx);
        int is_connected = connected;
        pthread_mutex_unlock(&mtx);
        if (is_connected) break;

        char buf[BUFFER_SIZE];
        struct sockaddr_in source;
        socklen_t source_len = sizeof(source);

        ssize_t n = recvfrom(udpid->sockfd, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&source, &source_len);
        if (n < 0)
        {
            if (errno == EINTR) continue;
            perror("recvfrom");
            return NULL;
        }
        buf[n] = '\0';

        if (!strcmp(buf, BROADCAST_MESSAGE))
        {
            char reply[BUFFER_SIZE];
            snprintf(reply, BUFFER_SIZE, "%s %s %s", TYPE.c_str(), ID.c_str(), name.c_str());
            if (sendto(udpid->sockfd, reply, strlen(reply), 0, (struct sockaddr*)&source, source_len) < 0)
                perror("sendto");
        }
    }
    return NULL;
}

void* connection_check(void* arg)
{
    Identity* tcpid = (Identity*)arg;

    if (listen(tcpid->sockfd, SOMAXCONN) < 0)
    {
        perror("listen");
        close(tcpid->sockfd);
        return NULL;
    }

    if (userfd < 0)
    {
        userfd = accept(tcpid->sockfd, (struct sockaddr*)&usr_addr, &usr_addr_len);
        if (userfd < 0)
        {
            perror("accept");
            close(tcpid->sockfd);
            return NULL;
        }
    }

    string recv_msg;
    recv_message(userfd, recv_msg);
    stringstream ss(recv_msg);
    string cmd, inp_pass;
    ss >> cmd >> inp_pass;
    if (cmd == "CONNECT")
    {
        if (inp_pass == pass)
        {
            pthread_mutex_lock(&mtx);
            connected = true;
            pthread_mutex_unlock(&mtx);
            string success_msg = to_string(SUCCESS_CONNECTION) + " Connection authorized";
            send_message(userfd, success_msg);
        }
        else
        {
            string error_msg = to_string(ERROR_PASSWORD_INCORRECT) + " Password is incorrect";
            send_message(userfd, error_msg);
            close(userfd);
            userfd = -1;
        }
    }
    else
    {
        close(userfd);
        userfd = -1;
    }

    return NULL;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./sensor [PORT]");
        return 0;
    }

    int inp_port = atoi(argv[1]);
    if (inp_port <= 0 || inp_port > 65535)
    {
        printf("Invalid port. Port must be between 0 and 65535");
        return 0;
    }
    uint16_t port = (uint16_t)inp_port;

    Identity* udpid = create_identity(port, SOCK_DGRAM, SOL_SOCKET, SO_REUSEADDR, INADDR_ANY);
    Identity* tcpid = create_identity(port, SOCK_STREAM, SOL_SOCKET, SO_REUSEADDR, INADDR_ANY);

    pthread_t s1, s2;
    string buf, cmd;
      
    while (1)
    {
        while (!connected)
        {
            if (pthread_create(&s1, NULL, broadcast_listening, udpid) != 0) 
            {
                perror("pthread_create s1");
                return 1;
            } 
            if (pthread_create(&s2, NULL, connection_check, tcpid) != 0) 
            {
                perror("pthread_create s2");
                return 1;
            }

            pthread_join(s2, NULL);
        }

        recv_message(userfd, buf);
        stringstream ss(buf);
        ss >> cmd;
        if (cmd == "CHANGE_PW")
        {
            string inp_current_pass, new_pass;
            ss >> inp_current_pass >> new_pass;
            if (inp_current_pass != pass)
            {
                string msg = to_string(ERROR_PASSWORD_INCORRECT) + " Password is incorrect";
                send_message(userfd, msg);
            }
            else
            {
                pass = new_pass;
                string msg = to_string(SUCCESS_PASSWORD_CHANGE) + " Password updated";
                cout << "Password updated to: " << new_pass << endl ;
                send_message(userfd, msg);
            }
        }
        if (cmd == "CONFIG")
        {
            string param;
            float value;
            ss >> param >> value;
            if (param != "T")
            {
                string msg = to_string(ERROR_INVALID_PARAM) + " No such param for sensor";
                send_message(userfd, msg);
            }
            else
            {
                T = value;
                cout << "T has been set to: " << T << endl;
                string msg = to_string(SUCCESS_PARAM_CHANGE) + " Sensor T value configured";
                send_message(userfd, msg);
            }
        }
    }
    delete udpid, tcpid;
    return 0;
}