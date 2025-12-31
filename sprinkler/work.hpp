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

void work(int v)
{
    cout << "SYS: Watering with V=" + to_string(v) << endl;
    logging("log/activity.txt", string("V=" + to_string(v)));
}

void work2()
{
    double H = randnumbers(1, 0, 100)[0];
    cout << "SYS: H=" + to_string(H) << endl;
    if (H < Hmin)
        cout << "SYS: Watering with V=" + to_string(V) << endl;
    logging("log/activity.txt", string("H=" + to_string(H) + " Watering=" + (H < Hmin ? "True" : "False") + " V=" + to_string(V)));
}

void* work2_thread(void* arg)
{
    (void)arg;
    pthread_mutex_lock(&m);
    while (true)
    {
        while (!power)
            pthread_cond_wait(&cnd, &m);

        cout << "Thread2 time calc" << endl;

        timespec t;
        clock_gettime(CLOCK_REALTIME, &t);
        t.tv_sec += 3;

        int rc = pthread_cond_timedwait(&cnd, &m, &t);
        if (!power || (rc == 0)) continue;

        pthread_mutex_unlock(&m);
        work2();
        pthread_mutex_lock(&m);
    }
    return nullptr;
}

void work1()
{
    double H = randnumbers(1, 0, 100)[0];
    cout << "SYS: H=" + to_string(H) << endl;
    if (H < Hmax)
        cout << "SYS: Watering with V=" + to_string(V) << endl;
    logging("log/activity.txt", string("H=" + to_string(H) + " Watering=" + (H < Hmax ? "True" : "False") + " V=" + to_string(V)));
}

void* work1_thread(void* arg)
{
    (void)arg;
    time_t last_trigger = 0;
    pthread_mutex_lock(&m);
    while (true)
    {
        while (!power || schedule.empty())
            pthread_cond_wait(&cnd, &m);

        cout << "Thread1 time calc" << endl;

        timespec now_ts;
        clock_gettime(CLOCK_REALTIME, &now_ts);

        struct tm tm_now;
        localtime_r(&now_ts.tv_sec, &tm_now);

        time_t next_trigger = 0;
        bool found = false;
        for (const auto& timestamp : schedule)
        {
            int h, m, s;
            if (sscanf(timestamp.c_str(), "%d:%d:%d", &h, &m, &s) != 3) continue;

            struct tm tm_target = tm_now;
            tm_target.tm_hour = h;
            tm_target.tm_min = m;
            tm_target.tm_sec = s;
            tm_target.tm_isdst = -1;

            time_t candidate = mktime(&tm_target);
            if (candidate < now_ts.tv_sec || (candidate == now_ts.tv_sec && now_ts.tv_nsec > 0))
                candidate += 24 * 60 * 60;
            if (candidate == last_trigger) candidate += 24 * 60 * 60;

            if (!found || candidate < next_trigger)
            {
                next_trigger = candidate;
                found = true;
            }
        }

        if (!found)
        {
            pthread_cond_wait(&cnd, &m);
            continue;
        }

        timespec t;
        t.tv_sec = next_trigger;
        t.tv_nsec = 0;
        int rc = pthread_cond_timedwait(&cnd, &m, &t);
        if (!power || schedule.empty()) continue;
        if (rc == ETIMEDOUT)
        {
            last_trigger = next_trigger;
            pthread_mutex_unlock(&m);
            work1();
            pthread_mutex_lock(&m);
        }
    }
    return nullptr;
}
