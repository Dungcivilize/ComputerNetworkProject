#pragma once

#include "framework.hpp"
#include "utils.hpp"

static vector<double> randnumbers(size_t len, double min = 0, double max = 1)
{
    vector<double> stats;
    random_device rd;
    mt19937 rng(rd());
    uniform_real_distribution<> dist(min, max);
    for (int count = 0; count < len; count++)
        stats.push_back(dist(rng));
    return stats;
}

void work()
{
    cout << "SYS: Evaluating nutrients." << endl;
    vector<double> stats = randnumbers(4, 0, 100);
    logging("log/activity.txt", string("H=" + to_string(stats[0]) + " N=" + to_string(stats[1]) + " P=" + to_string(stats[2]) + " K=" + to_string(stats[3])));
}

void* work_thread(void* arg)
{
    pthread_mutex_lock(&m);
    while (true)
    {
        while (!power)
            pthread_cond_wait(&cnd, &m);

        timespec t;
        clock_gettime(CLOCK_REALTIME, &t);
        t.tv_sec += T * 60;

        int rc = pthread_cond_timedwait(&cnd, &m, &t);
        if (!power || (rc == 0)) continue;

        pthread_mutex_unlock(&m);
        work();
        pthread_mutex_lock(&m);
    }
}