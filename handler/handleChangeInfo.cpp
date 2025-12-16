#include "handleChangeInfo.hpp"

void change_password(Device* device, string current_password, string new_password)
{
    string buf = "CHANGE_PW " + current_password + " " + new_password;
    send_message(device->sockfd, buf);
    recv_message(device->sockfd, buf);
    cout << buf << endl;
}

void change_param(Device* device, string param, float value)
{
    string buf = "CONFIG " + param + " " + to_string(value);
    send_message(device->sockfd, buf);
    recv_message(device->sockfd, buf);
    cout << buf << endl;
}