#include "framework.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

unordered_map<string, string> credentials;
string id = randstr(16);
string password = "light";
const string type = "light";
int power = 0;
int P = 10;
int S = 5;
vector<string> schedule;
ofstream file;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cnd = PTHREAD_COND_INITIALIZER;

void work()
{
    cout << "SYS: Light on with P = " + to_string(P) + " for " + to_string(S) + "s" << endl;
    vector<double> stats = randnumbers(4, 0, 100);
    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);

    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    string timestamp(buf);

    file << timestamp + " P=" + to_string(P) + " S=" + to_string(S) << endl; 
}

// void* work_thread(void* arg);

bool check_credential(unordered_map<string, string>& credentials, const string& token)
{
    auto it = credentials.find(token);
    return !(it == credentials.end());
}

int main(int argc, char* argv[])
{
    file.open("activity_log.txt");
    cout << "Light running:\nID: " + id + "\nPassword: " + password << endl;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&sa, sizeof(sa)) < 0)
    {
        perror("bind");
        close(server_fd);
        return 1;
    }
    if (listen(server_fd, SOMAXCONN) < 0)
    {
        perror("listen");
        close(server_fd);
        return 1;
    }

    /*
    pthread_t background;
    if (pthread_create(&background, nullptr, work_thread, NULL) != 0)
    {
        perror("pthread_create");
        close(server_fd);
        return 1;
    }
    */

    vector<pollfd> pfds;
    pfds.push_back({server_fd, POLLIN, 0});   

    while (true)
    {
        int rc = poll(pfds.data(), pfds.size(), -1);
        if (rc < 0)
        {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }

        if (pfds[0].revents & POLLIN)
        {
            sockaddr_in client{};
            socklen_t client_len = sizeof(client);
            int client_fd = accept(server_fd, (sockaddr*)&client, &client_len);
            if (client_fd >= 0)
                pfds.push_back({client_fd, POLLIN, 0});
        }

        for (size_t i = 1; i < pfds.size();)
        {
            short revents = pfds[i].revents;
            bool closefd = false;
            if (revents & (POLLHUP | POLLERR | POLLNVAL))
            {
                close(pfds[i].fd);
                pfds.erase(pfds.begin() + i);
                continue;
            }
            if (revents & POLLIN)
            {
                string buf;
                if (!recv_message(pfds[i].fd, buf))
                {
                    close(pfds[i].fd);
                    pfds.erase(pfds.begin() + i);
                    continue;
                }

                stringstream ss(buf);
                string cmd;
                ss >> cmd;

                string reply;
                if (cmd == "SCAN")
                {
                    cout << to_string(SUCCESS_SCAN) + ": Responded to a scan signal" << endl;
                    reply = "SCAN " + to_string(SUCCESS_SCAN) + " " + id + ":" + type;
                    closefd = true;
                }           
                else if (cmd == "CONNECT")
                {
                    string appid, in_password;
                    ss >> appid >> in_password;
                    if (password == in_password)
                    {
                        string token = randstr(TOKEN_SIZE);
                        credentials[token] = appid;
                        reply = "CONNECT " + to_string(SUCCESS_CONNECTION) + " " + token + ":" + id + ":" + type;
                        cout << to_string(SUCCESS_CONNECTION) + ": Successful connection from: " + appid << endl;
                    }
                    else
                    {
                        closefd = true;
                        reply = "CONNECT " + to_string(ERROR_PW_INCORRECT) + " Password incorrect";
                        cout << to_string(ERROR_PW_INCORRECT) + ": Refuse connection due to incorrect password" << endl;
                    }
                }
                else if (cmd == "CHANGE_PW")
                {
                    string token, current_pass, new_pass;
                    ss >> token >> current_pass >> new_pass;
                    if (!check_credential(credentials, token))
                    {
                        reply = "CHANGE_PW " + to_string(ERROR_INVALID_TOKEN) + " Invalid token";
                        cout << to_string(ERROR_INVALID_TOKEN) + ": Cannot identify received credential token" << endl;
                    }
                    else if (current_pass != password)
                    {
                        reply = "CHANGE_PW " + to_string(ERROR_PW_INCORRECT) + " Password incorrect";
                        cout << to_string(ERROR_PW_INCORRECT) + ": Password change failed due to incorrect password. Request from: " + credentials[token] << endl;
                    }
                    else
                    {
                        password = new_pass;
                        reply = "CHANGE_PW " + to_string(SUCCESS_PW_CHANGE) + " OK";
                        cout << to_string(SUCCESS_PW_CHANGE) + ": Password changed to \"" + password + "\". Request from: " + credentials[token] << endl;
                    }
                }
                else if (cmd == "QUERY")
                {
                    string token, scope;
                    ss >> token >> scope;
                    if (!check_credential(credentials, token))
                    {
                        reply = "QUERY " + to_string(ERROR_INVALID_TOKEN) + " Invalid token";
                        cout << to_string(ERROR_INVALID_TOKEN) + ": Cannot identify received credential token" << endl;
                    }
                    else if (scope != "STATE" && scope != "PARAM" && scope != "SCHEDULE" && scope != "ALL")
                    {
                        reply = "QUERY " + to_string(ERROR_INVALID_SCOPE) + " Invalid scope";
                        cout << to_string(ERROR_INVALID_SCOPE) + ": Invalid scope received: " + scope + ". Request from: " + credentials[token] << endl;   
                    }
                    else
                    {
                        string message;
                        string display_schedule;
                        if (schedule.empty()) display_schedule = "NaN";
                        else
                            for (size_t idx = 0; idx < schedule.size(); idx++)
                                display_schedule += (idx == schedule.size() - 1 ? schedule[idx] : (schedule[idx] + ":"));
                        if (scope == "STATE") message = to_string(power);
                        else if (scope == "PARAM") message = to_string(P) + ":" + to_string(S);
                        else if (scope == "SCHEDULE") message = display_schedule;
                        else message = to_string(power) + ":" + to_string(P) + ":" + to_string(S) + ":" + display_schedule;
                        reply = "QUERY " + to_string(SUCCESS_QUERY) + " " + message;
                        cout << to_string(SUCCESS_QUERY) + ": Query OK. Request from: " + credentials[token] << endl;
                    }
                }
                else if (cmd == "CONFIG")
                {
                    string token, param;
                    int value;
                    ss >> token >> param >> value;
                    if (!check_credential(credentials, token))
                    {
                        reply = "CONFIG " + to_string(ERROR_INVALID_TOKEN) + " Invalid token";
                        cout << to_string(ERROR_INVALID_TOKEN) + ": Cannot identify received credential token" << endl;
                    }
                    else if (param != "P" && param != "S")
                    {
                        reply = "CONFIG " + to_string(ERROR_INVALID_PARAM) + " Unknown parameter";
                        cout << to_string(ERROR_INVALID_PARAM) + ": Unknown parameter. Request from: " + credentials[token] << endl;
                    }
                    else if (value <= 0)
                    {
                        reply = "CONFIG " + to_string(ERROR_INVALID_VALUE) + " Value must greater than 0";
                        cout << to_string(ERROR_INVALID_VALUE) + ": Invalid value received. Request from: " + credentials[token] << endl;
                    }
                    else
                    {
                        pthread_mutex_lock(&m);
                        if (param == "P") P = value; else S = value;
                        pthread_cond_signal(&cnd);
                        pthread_mutex_unlock(&m);
                        reply = "CONFIG " + to_string(SUCCESS_CONFIG) + " OK";
                        cout << to_string(SUCCESS_CONFIG) + ": " + param + " has been set to " + to_string(P) + ". Request from: " + credentials[token] << endl;
                    }
                }
                else if (cmd == "POWER")
                {
                    string token, state;
                    ss >> token >> state;
                    if (!check_credential(credentials, token))
                    {
                        reply = "POWER " + to_string(ERROR_INVALID_TOKEN) + " Invalid token";
                        cout << to_string(ERROR_INVALID_TOKEN) + ": Cannot identify received credential token" << endl;
                    }
                    else if ((state == "ON" && power) || (state == "OFF" && !power) || (state != "ON" && state != "OFF"))
                    {
                        reply = "POWER " + to_string(ERROR_INVALID_VALUE) + " Invalid state";
                        cout << to_string(ERROR_INVALID_VALUE) + ": Invalid state. Request from: " + credentials[token] << endl;
                    }
                    else
                    {
                        pthread_mutex_lock(&m);
                        power = state == "ON" ? 1 : 0;
                        pthread_cond_signal(&cnd);
                        pthread_mutex_unlock(&m);
                        reply = "POWER " + to_string(SUCCESS_COMMAND) + " OK";
                        cout << to_string(SUCCESS_COMMAND) + ": Power turned " + (power ? "on" : "off") + ". Request from: " + credentials[token] << endl;
                    }
                }
                else if (cmd == "LIGHT")
                {
                    string token;
                    ss >> token;
                    if (!check_credential(credentials, token))
                    {
                        reply = "LIGHT " + to_string(ERROR_INVALID_TOKEN) + " Invalid token";
                        cout << to_string(ERROR_INVALID_TOKEN) + ": Cannot identify received credential token" << endl;
                    }
                    else
                    {
                        reply = "LIGHT " + to_string(SUCCESS_COMMAND) + " Lights ON";
                        cout << to_string(SUCCESS_COMMAND) + ": Lights turn on. Request from: " + credentials[token] << endl;
                        work();
                    }
                }
                else
                {
                    reply = cmd + " " + to_string(ERROR_UNSUPPORT_PROTOCOL) + " This protocol is not supported";
                    cout << to_string(ERROR_UNSUPPORT_PROTOCOL) + ": Unknown protocol received.";
                }
                if (!send_message(pfds[i].fd, reply) || closefd)
                {
                    close(pfds[i].fd);
                    pfds.erase(pfds.begin() + i);
                    continue;
                }
            }
            pfds[i].revents = 0;
            i++;
        }
    }
    file.close();
    for (size_t i = 0; i < pfds.size(); i++)
        close(pfds[i].fd);
    return 0;
}