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

void work(int c, int v)
{
    cout << "SYS: Fertilizing with C=" + to_string(c) + " V=" + to_string(v)<< endl;
    logging("log/activity.txt", string("V=" + to_string(c) + " V=" + to_string(v)));
}

void work2()
{
    vector nutrients = randnumbers(3, 0, 100);
    cout << "SYS: N=" + to_string(nutrients[0]) + " P=" + to_string(nutrients[1]) + " K=" + to_string(nutrients[2]) << endl;
    if (nutrients[0] < Nmin || nutrients[1] < Pmin || nutrients[2] < Kmin)
        cout << "SYS: Fertilizing with C=" + to_string(C) + " V=" + to_string(V)<< endl;
    logging("log/activity.txt", string("N=" + to_string(nutrients[0]) + " P=" + to_string(nutrients[1]) + " K=" + to_string(nutrients[2]) + " Fertilizing=" + (nutrients[0] < Nmin || nutrients[1] < Pmin || nutrients[2] < Kmin ? "True" : "False") + " C=" + to_string(C) + "V=" + to_string(V)));
}

void* work2_thread(void* arg)
{
    (void)arg;
    pthread_mutex_lock(&m);
    while (true)
    {
        while (!power)
            pthread_cond_wait(&cnd, &m);

        timespec t;
        clock_gettime(CLOCK_REALTIME, &t);
        t.tv_sec += 3;

        int rc = pthread_cond_timedwait(&cnd, &m, &t);
        if (!power || (rc == 0)) continue;

        pthread_mutex_unlock(&m);
        work2();
        pthread_mutex_lock(&m);
    }
}