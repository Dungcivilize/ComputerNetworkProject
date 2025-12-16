#include "../common/index.hpp"
#include "handleConnect.hpp"

Device* establish_connection(DeviceInfo info, string password)
{
    int devfd = socket(AF_INET, SOCK_STREAM, 0);
    if (devfd < 0)
    {
        perror("establish_connection.socket");
        return NULL;
    }
    struct sockaddr_in devaddr;
    devaddr.sin_family = AF_INET;
    devaddr.sin_addr = info.addr.sin_addr;
    devaddr.sin_port = info.addr.sin_port;
    if (connect(devfd, (struct sockaddr*)&devaddr, sizeof(devaddr)) < 0)
    {
        perror("establish_connection.connect");
        return NULL;
    }
    string buf = "CONNECT " + password;
    send_message(devfd, buf);
    recv_message(devfd, buf);
    cout << buf << endl;
    stringstream ss(buf);
    int code = 0;
    ss >> code;
    if (code == SUCCESS_CONNECTION)
        return new Device(info, devfd);
    else
        close(devfd);
    return NULL;
}