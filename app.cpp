#include "dependencies/index.hpp"
#include "common/index.hpp"
#include "handler/handlecommand.hpp"

int main(int argc, char** argv)
{
    int inp_port = check_args(argc, argv);
    if (inp_port < 0)
        return 1;

    int app_id = register_app();
    printf("App registered with ID: %d\n", app_id);

    uint16_t port = (uint16_t)inp_port;
    vector<DeviceInfo> device_list;
    vector<Device*> devices;
    while (1)
    {
        fd_set rfds;
        FD_ZERO(&rfds);

        FD_SET(STDIN_FILENO, &rfds);
        int maxfd = STDIN_FILENO;

        for (auto* device : devices)
        {
            FD_SET(device->sockfd, &rfds);
            if (device->sockfd > maxfd)
                maxfd = device->sockfd;
        }

        int ready = select(maxfd + 1, &rfds, NULL, NULL, NULL);
        if (ready < 0)
        {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &rfds))
        {
            handleCommand(devices, port);
        }
        for (size_t idx = 0; idx < devices.size();)
        {
            Device* dev = devices[idx];

            if (FD_ISSET(dev->sockfd, &rfds))
            {
                string buf;
                ssize_t n = recv_message(dev->sockfd, buf);
                if (n <= 0)
                {
                    if (n < 0)
                        perror("recv_message");
                    cout << "Device " << dev->info.id << " disconnected.";

                    delete dev;
                    devices.erase(devices.begin() + idx);
                    continue;
                }
                else
                {
                    cout << buf << endl;
                }
            }
            idx++;
        }
    }

    for (auto* dev : devices)
        delete dev;
    return 0;
}