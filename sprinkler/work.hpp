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

double sample_humidity() { return randnumbers(1, 0, 100)[0]; }

static time_t midnight_time(time_t t)
{
    struct tm tm_time;
    localtime_r(&t, &tm_time);
    tm_time.tm_hour = 0;
    tm_time.tm_min = 0;
    tm_time.tm_sec = 0;
    tm_time.tm_isdst = -1;
    return mktime(&tm_time);
}

static int count_schedule_crosses(const vector<string>& schedule, time_t prev_time, time_t now_time)
{
    if (schedule.empty() || prev_time <= 0 || now_time <= prev_time) return 0;

    int count = 0;
    time_t day = midnight_time(prev_time);
    time_t end_day = midnight_time(now_time);

    while (day <= end_day)
    {
        struct tm day_tm;
        localtime_r(&day, &day_tm);

        for (const auto& timestamp : schedule)
        {
            int h, m, s;
            if (sscanf(timestamp.c_str(), "%d:%d:%d", &h, &m, &s) != 3) continue;

            struct tm target_tm = day_tm;
            target_tm.tm_hour = h;
            target_tm.tm_min = m;
            target_tm.tm_sec = s;
            target_tm.tm_isdst = -1;

            time_t candidate = mktime(&target_tm);
            if (candidate > prev_time && candidate <= now_time) count++;
        }

        struct tm next_tm = day_tm;
        next_tm.tm_mday += 1;
        next_tm.tm_hour = 0;
        next_tm.tm_min = 0;
        next_tm.tm_sec = 0;
        next_tm.tm_isdst = -1;
        time_t next_day = mktime(&next_tm);
        if (next_day <= day) break;
        day = next_day;
    }

    return count;
}

void* work_thread(void* arg)
{
    (void)arg;
    time_t last_sample = 0;
    pthread_mutex_lock(&m);
    while (true)
    {
        while (!power)
        {
            pthread_cond_wait(&cnd, &m);
            last_sample = 0;
        }

        if (last_sample == 0)
        {
            timespec now_ts;
            clock_gettime(CLOCK_REALTIME, &now_ts);
            last_sample = now_ts.tv_sec;
        }

        timespec t;
        clock_gettime(CLOCK_REALTIME, &t);
        t.tv_sec += 3;

        int rc = pthread_cond_timedwait(&cnd, &m, &t);
        if (!power || (rc == 0)) continue;

        timespec now_ts;
        clock_gettime(CLOCK_REALTIME, &now_ts);
        time_t now_time = now_ts.tv_sec;
        int local_Hmin = Hmin;
        int local_Hmax = Hmax;
        int local_V = V;
        vector<string> local_schedule = schedule;

        pthread_mutex_unlock(&m);

        double H = sample_humidity();
        if (H < local_Hmin) work(local_V);

        int crossed = count_schedule_crosses(local_schedule, last_sample, now_time);
        if (H < local_Hmax)
        {
            for (int i = 0; i < crossed; i++)
                work(local_V);
        }

        last_sample = now_time;

        pthread_mutex_lock(&m);
    }
}
