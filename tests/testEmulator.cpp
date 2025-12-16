#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <string>
#include <arpa/inet.h>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <port> <count>" << endl;
        return 1;
    }

    int port = atoi(argv[1]);
    int count = atoi(argv[2]);

    // Get BASE_IP
    FILE* fp = popen("hostname -I | awk '{print $1}'", "r");
    if (!fp) {
        cout << "Failed to get BASE_IP" << endl;
        return 1;
    }
    char base_ip[20];
    if (fgets(base_ip, sizeof(base_ip), fp) == NULL) {
        cout << "Failed to read BASE_IP" << endl;
        pclose(fp);
        return 1;
    }
    pclose(fp);
    base_ip[strcspn(base_ip, "\n")] = 0; // remove newline

    string base = base_ip;
    size_t last_dot = base.rfind('.');
    if (last_dot == string::npos) {
        cout << "Invalid BASE_IP format" << endl;
        return 1;
    }
    string prefix = base.substr(0, last_dot + 1); // include the dot

    string msg = "test message";

    for (int i = 1; i <= count; ++i) {
        string ip = prefix + to_string(i);
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            cout << ip << ": Thất bại (socket error)" << endl;
            continue;
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        addr.sin_port = htons(port);

        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            send(sock, msg.c_str(), msg.size(), 0);
            char buffer[1024] = {0};
            int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (n > 0) {
                string response(buffer, n);
                if (response == msg) {
                    cout << ip << ": Thành công" << endl;
                } else {
                    cout << ip << ": Thất bại (phản hồi sai)" << endl;
                }
            } else {
                cout << ip << ": Thất bại (không nhận phản hồi)" << endl;
            }
        } else {
            cout << ip << ": Thất bại (không kết nối)" << endl;
        }
        close(sock);
    }

    return 0;
}