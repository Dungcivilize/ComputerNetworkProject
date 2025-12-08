#define NAME_PLACEHOLDER "UNNAMED DEVICE"

#include "dependencies/framework.hpp"

typedef struct
{
    int hour;
    int min;
    int sec;
} Time;

class Device
{
public:
    int sockfd;
    string id;
    string name;

    Device(int sockfd, const string& id, const string& name = NAME_PLACEHOLDER) : id(id), sockfd(sockfd), name(name) {}

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    virtual ~Device() { close(this->sockfd); }
};

class Sensor : public Device
{
public:
    double T;

    Sensor(int sockfd , const string& id, const string& name = NAME_PLACEHOLDER, double T = 10.0) : Device(sockfd, id, name), T(T) {}

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
};

class Sprinkler : public Device
{
public:
    double Hmin;
    double Hmax;
    vector<Time> times;

    Sprinkler(int sockfd, const string& id, const string& name = NAME_PLACEHOLDER, double Hmin = 1.0, double Hmax = 10.0) : Device(sockfd, id, name), Hmin(Hmin), Hmax(Hmax), times() {}

    bool watering()
    {
        
    }
};