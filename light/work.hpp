#pragma once

#include "framework.hpp"
#include "utils.hpp"

void work(int p, int s)
{
    cout << "SYS: Light on with P = " + to_string(p) + " for " + to_string(s) + "s" << endl;
    logging("log/activity.txt", string("P=" + to_string(p) + " S=" + to_string(s)));
}

void* work_thread(void* arg)
{
    (void)arg;
    time_t last_trigger = 0;
    pthread_mutex_lock(&m);
    while (true)
    {
        while (!power || schedule.empty())
            pthread_cond_wait(&cnd, &m);

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
            work(P, S);
            pthread_mutex_lock(&m);
        }
    }
}
