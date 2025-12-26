#include "framework.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"

bool check_credential(unordered_map<string, string>& credentials, const string& token)
{
    auto it = credentials.find(token);
    return !(it == credentials.end());
}

int main(int argc, char* argv[])
{
    unordered_map<string, string> credentials;
    int T = 1000;
    string id = randstr(16);
    string name = "unnamed sensor";
    string password = "sensor";
    const string type = "SENSOR";
    cout << "Sensor running:\nID: " + id + "\nName: " + name + "\nPassword: " + password << endl;

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
                    reply = "SCAN " + to_string(SUCCESS_SCAN) + " " + id + ":" + name + ":" + type;
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
                        reply = "CONNECT " + to_string(SUCCESS_CONNECTION) + " " + id + ":" + name + ":" + type + ":" + token + ":" + to_string(T);
                    }
                    else
                    {
                        closefd = true;
                        reply = "CONNECT " + to_string(ERROR_PW_INCORRECT) + " Password incorrect";
                    }
                }
                else if (cmd == "CHANGE_PW")
                {
                    string token, current_pass, new_pass;
                    ss >> token >> current_pass >> new_pass;
                    if (!check_credential(credentials, token))
                        reply = "CHANGE_PW " + to_string(ERROR_INVALID_TOKEN) + " Invalid token";
                    else if (current_pass != password)
                        reply = "CHANGE_PW " + to_string(ERROR_PW_INCORRECT) + " Password incorrect";
                    else
                    {
                        password = new_pass;
                        reply = "CHANGE_PW " + to_string(SUCCESS_PW_CHANGE) + " OK";
                    }
                    
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

    for (size_t i = 0; i < pfds.size(); i++)
        close(pfds[i].fd);
    return 0;
}