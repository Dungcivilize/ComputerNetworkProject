#include "framework.hpp"
#include "streamtransmission.hpp"
#include "utils.hpp"
#include "work.hpp"
#include "handler/scan.hpp"
#include "handler/connect.hpp"
#include "handler/changepassword.hpp"
#include "handler/config.hpp"
#include "handler/query.hpp"
#include "handler/power.hpp"

int main(int argc, char* argv[])
{
    cout << "Sensor running:\nID: " + id + "\nPassword: " + password << endl;

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

    pthread_t background;
    if (pthread_create(&background, nullptr, work_thread, NULL) != 0)
    {
        perror("pthread_create");
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

                string cmd, content, reply;
                parse_message(buf, cmd, content);

                if (cmd == "SCAN")
                {
                    handle_scan(content, reply);
                    closefd = true;
                }           
                else if (cmd == "CONNECT")
                    handle_connect(content, reply, closefd);
                else if (cmd == "CHANGE_PW")
                    handle_change_password(content, reply);
                else if (cmd == "QUERY")
                    handle_query(content, reply);
                else if (cmd == "CONFIG")
                    handle_config(content, reply);
                else if (cmd == "POWER")
                    handle_power(content, reply);
                else
                    reply = cmd + " " + to_string(ERROR_UNSUPPORT_PROTOCOL) + " This protocol is not supported";
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