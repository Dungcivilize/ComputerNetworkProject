#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <errno.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <fcntl.h>
#include <thread>
#include <atomic>
#include <chrono>

using namespace std;

// Non-blocking connect with timeout (ms). Returns socket fd or -1.
int connect_to(const string &ip, uint16_t port, int timeout_ms = 20)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
        close(sock);
        return -1;
    }

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) flags = 0;
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(sock);
        return -1;
    }

    int res = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (res == 0) {
        fcntl(sock, F_SETFL, flags);
    } else if (errno == EINPROGRESS) {
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(sock, &wfds);
        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        int sel = select(sock + 1, NULL, &wfds, NULL, &tv);
        if (sel <= 0) { close(sock); return -1; }
        int so_error = 0; socklen_t len = sizeof(so_error);
        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) { close(sock); return -1; }
        if (so_error != 0) { close(sock); return -1; }
        fcntl(sock, F_SETFL, flags);
    } else {
        close(sock); return -1;
    }

    // small recv/send timeouts
    struct timeval tv2; tv2.tv_sec = 0; tv2.tv_usec = timeout_ms * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv2, sizeof tv2);
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv2, sizeof tv2);
    return sock;
}

bool send_line(int sock, const string &msg)
{
    string s = msg + "\n";
    ssize_t n = send(sock, s.c_str(), s.size(), 0);
    return n == (ssize_t)s.size();
}

// recv a line with a small total timeout (ms)
string recv_line(int sock, int timeout_ms)
{
    string res; char c;
    using clock = chrono::steady_clock;
    auto deadline = clock::now() + chrono::milliseconds(timeout_ms);
    while (clock::now() < deadline) {
        auto rem = chrono::duration_cast<chrono::milliseconds>(deadline - clock::now()).count();
        if (rem <= 0) break;
        fd_set rfds; FD_ZERO(&rfds); FD_SET(sock, &rfds);
        struct timeval tv; tv.tv_sec = rem/1000; tv.tv_usec = (rem%1000)*1000;
        int sel = select(sock + 1, &rfds, NULL, NULL, &tv);
        if (sel <= 0) break;
        ssize_t n = recv(sock, &c, 1, 0);
        if (n <= 0) break;
        if (c == '\n') break;
        res.push_back(c);
    }
    return res;
}

// get local IPv4 address by opening a UDP socket to a public IP
bool get_local_ip(string &out)
{
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) return false;
    struct sockaddr_in srv; memset(&srv,0,sizeof(srv));
    srv.sin_family = AF_INET; srv.sin_port = htons(53);
    inet_pton(AF_INET, "8.8.8.8", &srv.sin_addr);
    connect(s, (struct sockaddr*)&srv, sizeof(srv));
    struct sockaddr_in name; socklen_t namelen = sizeof(name);
    if (getsockname(s, (struct sockaddr*)&name, &namelen) == 0) {
        char buf[INET_ADDRSTRLEN]; inet_ntop(AF_INET, &name.sin_addr, buf, sizeof(buf)); out = buf; close(s); return true;
    }
    close(s); return false;
}

int main(int argc, char** argv)
{
    uint16_t port = 9000; if (argc >= 2) port = (uint16_t)atoi(argv[1]);
    string local; if (!get_local_ip(local)) { cerr<<"Cannot detect local IP\n"; return 1; }
    array<int,4> octets; char dot; stringstream ss(local);
    int a,b,c,d; char ch; ss>>a>>ch>>b>>ch>>c>>ch>>d; string base = to_string(a)+"."+to_string(b)+"."+to_string(c);
    int mylast = d;
    cout<<"Local: "<<local<<" scanning "<<base<<".1-254 port "<<port<<"\n";

    // prepare list
    vector<int> targets; targets.reserve(254);
    for (int i=1;i<=254;++i) if (i!=mylast) targets.push_back(i);

    atomic<size_t> idx{0}; size_t total = targets.size();
    unsigned int workers = thread::hardware_concurrency(); if (workers==0) workers = 8; workers = min<unsigned int>(workers*4, 128);

    auto worker = [&](int tid){
        while (true) {
            size_t i = idx.fetch_add(1);
            if (i >= total) break;
            int last = targets[i]; string ip = base + "." + to_string(last);
            int s = connect_to(ip, port, 20);
            if (s < 0) continue;
            send_line(s, "1");
            string r = recv_line(s, 20);
            if (!r.empty()) cout << "Found: "<< ip << " -> " << r << "\n";
            close(s);
        }
    };

    vector<thread> th; th.reserve(workers);
    for (unsigned int t=0;t<workers;++t) th.emplace_back(worker, t);
    for (auto &t: th) t.join();

    cout<<"Scan complete."<<endl;
    return 0;
}