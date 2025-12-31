#ifndef SENSORSERVER_SENSORDATASTRUCTURE_HPP
#define SENSORSERVER_SENSORDATASTRUCTURE_HPP

#include <string>
#include <ctime>
#include <iostream>
#include <cmath>
using namespace std;

class SensorDataStructure {
    public:
        bool powered_on;
        bool timer_set; 
        bool timer_set_to;
        time_t timer_time;
        bool is_running_on_command;
        int t;
        float total_energy_consumed = 0.0; // đơn vị: kWh
        string type = "sensor";
        SensorDataStructure() : powered_on(false), timer_set(false), timer_set_to(false), timer_time(0), is_running_on_command(false), t(1) {}
        virtual ~SensorDataStructure() {}
        virtual int calculate_base_power() {
            int power_consumption = 0; // đơn vị: Watt

            time_t current_time = time(nullptr) % 86400 + 7 * 3600; // adjust to GMT+7

            // Nếu đang chạy chế độ hẹn giờ
            if (timer_set && current_time < timer_time) {
                power_consumption += 2; // bộ hẹn giờ
            }

            if (powered_on) {
                power_consumption += 5; // mạch điều khiển
            } else {
                return power_consumption;
            }
            return power_consumption; // tổng công suất tức thời (W)
        }
        virtual int calculate_running_on_command_power() {
            return 0; // base class has no additional power when running on command
        }
        int calculate_power() {
            return calculate_base_power() + calculate_running_on_command_power();
        }
        virtual float calculate_power_consumption(int duration_minutes, int running_on_command_minutes) {
            cout << "Checking: " << calculate_base_power() << " W for " << duration_minutes << " minutes and "
                 << calculate_running_on_command_power() << " W for " << running_on_command_minutes << " minutes." << endl;
            return (calculate_base_power() * duration_minutes + calculate_running_on_command_power() * running_on_command_minutes) / 10000.0; // kWh
        }
        virtual void save_config() {
            // Base class has no config to save
        }
        virtual void run(time_t start_time, time_t end_time) {
            // Base class has no scheduled actions
            int duration_minutes = (end_time - start_time) / 60;
            // Tính điện năng tiêu thụ 
            float energy_used = calculate_power_consumption(duration_minutes, 0);
            total_energy_consumed += energy_used;
        }
        virtual string toString()
        {
            return "";
        }
};

class SprinklerDataStructure : public SensorDataStructure {
    public:
        bool activate = false;
        bool pumping = false;
        float humidity;
        float min_humidity;
        float max_humidity;
        int volume_per_minute;
        int volume_per_humidity_percent;
        int command_volume = 0;
        int base_volume;
        int tank_capacity;
        int current_water_amount;
        time_t watering_start_time;
        time_t watering_end_time;
        SprinklerDataStructure(string id) : SensorDataStructure() {
            type = "sprinkler";
            // Đọc từ file config dựa theo id của sprinkler
            // Mở file config
            this->id = id;
            string path = "config/sprinkler_config_" + id + ".txt";
            FILE* file = fopen(path.c_str(), "r");
            if (file == nullptr) {
                // Nếu không mở được file, đặt các giá trị mặc định
                min_humidity = 30.0;
                max_humidity = 70.0;
                volume_per_minute = 10;
                volume_per_humidity_percent = 5;
                base_volume = 50;
                tank_capacity = 1000;
                watering_start_time = 6 * 3600; // 6 AM
                watering_end_time = 18 * 3600; // 6 PM
                t = 1; // 1 minute
                // Lưu các giá trị mặc định vào file
                save_config();
            } else {
                // Đọc các thông số từ file
                fscanf(file, "%f %f %d %d %d %d %ld %ld %d", &min_humidity, &max_humidity, &volume_per_minute, &volume_per_humidity_percent, &base_volume, &tank_capacity, (long*)&watering_start_time, (long*)&watering_end_time, &t);
                fclose(file);
            }
            current_water_amount = tank_capacity / 2;
            humidity = min_humidity;
        }
        int calculate_base_power() override {
            int power_consumption = 0; // đơn vị: Watt

            time_t current_time = time(nullptr) % 86400 + 7 * 3600; // adjust to GMT+7
            // Nếu đang chạy chế độ hẹn giờ
            if (timer_set && current_time < timer_time) {
                power_consumption += 2; // bộ hẹn giờ
            }

            if (powered_on) {
                power_consumption += 5; // mạch điều khiển
            } else {
                return power_consumption;
            }


            // Nếu đang tưới trong khoảng thời gian định sẵn
            if (activate) {
                power_consumption += 2 * volume_per_minute; // bơm tưới
            }

            // Nếu bơm đang nạp nước vào bể
            if (pumping) {
                // Công suất máy bơm vào bể cố định ở mức 50W
                power_consumption += 50; // bơm nước vào bể 
            }
            return power_consumption; // tổng công suất tức thời (W)
        }
        int calculate_running_on_command_power() override {
            return powered_on && is_running_on_command ? 2 * volume_per_minute : 0;
        }

        void save_config() override {
            // Lưu các thông số vào file config
            string path = "config/sprinkler_config_" + id + ".txt";
            FILE* file = fopen(path.c_str(), "w");
            if (file != nullptr) {
                fprintf(file, "%f %f %d %d %d %d %ld %ld %d\n", min_humidity, max_humidity, volume_per_minute, volume_per_humidity_percent, base_volume, tank_capacity, (long)watering_start_time, (long)watering_end_time, t);
                fclose(file);
            }
        }
        void run(time_t start_time, time_t end_time) override {
            // Tính thời gian hoạt động
            int duration_minutes = (end_time - start_time) / 60;

            int auto_watering_minutes = 0;
            time_t current_time = time(nullptr) % 86400 + 7 * 3600; // adjust to GMT+7
            // Nếu độ ẩm hiện tại thấp hơn mức tối thiểu hoặc trạng thái là đang tưới
            if (humidity < min_humidity) {
                // Kiểm tra xem có trong khoảng thời gian tưới định sẵn không
                if (watering_start_time <= watering_end_time) {
                    // Khoảng thời gian không qua nửa đêm
                    if (current_time >= watering_start_time && current_time <= watering_end_time) {
                        activate = true;
                    }
                } else {
                    // Khoảng thời gian qua nửa đêm
                    if (current_time >= watering_start_time || current_time <= watering_end_time) {
                        activate = true;
                    }
                }
            } else if (humidity >= max_humidity) {
                activate = false;
            }
            if (activate) {
                auto_watering_minutes = duration_minutes;
            }

            // Nếu lượng nước hiện tại trong bể thấp hơn 10%
            if (current_water_amount < tank_capacity / 10) {
                pumping = true;
            } else if (current_water_amount >= tank_capacity) {
                pumping = false;
            }
            if (pumping) {
                // Tính lượng nước bơm vào bể
                int water_added = 2 * base_volume * duration_minutes;
                current_water_amount += water_added;
            }

            int running_on_command_minutes = is_running_on_command ? command_volume / volume_per_minute : 0;
            if (running_on_command_minutes > duration_minutes) {
                running_on_command_minutes = duration_minutes;
                command_volume -= running_on_command_minutes * volume_per_minute;
            } else {
                command_volume = 0;
                is_running_on_command = false;
            }
            // Tính điện năng tiêu thụ 
            float energy_used = calculate_power_consumption(duration_minutes, running_on_command_minutes);
            total_energy_consumed += energy_used;

            // Kiểm tra timer 
            bool timer_triggered = false;
            if (timer_set && current_time >= timer_time) {
                // Thực hiện thay đổi trạng thái theo timer
                powered_on = timer_set_to;
                timer_set = false; // reset timer
                timer_triggered = true;
            }

            // Tính độ thay đổi độ ẩm theo thời gian trong ngày, ban ngày độ ẩm giảm nhanh hơn ban đêm
            float humidity_change_rate = (start_time % 86400 + 7 * 3600 >= 21600 && start_time % 86400 + 7 * 3600 <= 64800) ? -0.15 : -0.05; // từ 6h đến 18h
            humidity += humidity_change_rate * duration_minutes;
            if (humidity < 0) humidity = 0;

            // Nếu đang tưới nước
            if (activate || is_running_on_command) {
                // Tính lượng nước đã tưới
                int water_used = volume_per_minute * (auto_watering_minutes + running_on_command_minutes);
                if (water_used > current_water_amount) {
                    water_used = current_water_amount;
                }
                current_water_amount -= water_used;
                // Tăng độ ẩm dựa trên lượng nước đã tưới
                humidity += (float)water_used / volume_per_humidity_percent;
                if (humidity > 100) humidity = 100;
            }
            if (current_water_amount < 0) {
                current_water_amount = 0;
            }
            if (current_water_amount > tank_capacity) {
                current_water_amount = tank_capacity;
            }
            cout << "Sprinkler " << id << " run from " << ctime(&start_time) << " to " << ctime(&end_time)
                 << " | Power: " << calculate_power() << " W"
                 << " | Energy used: " << energy_used << " kWh"
                 << " | Humidity: " << humidity << "%"
                 << " | Water left: " << current_water_amount << " Liters" << endl;
            if (timer_triggered) {
                cout << "  (Timer triggered: Power set to " << (powered_on ? "ON" : "OFF") << ")" << endl;
            }
        }
        string toString() override
        {
            return to_string(volume_per_minute) + " " + to_string(humidity) + " " + to_string(min_humidity) + " " + to_string(max_humidity) + " " + to_string(watering_start_time) + " " + to_string(watering_end_time) + " " + to_string(current_water_amount) + " " + to_string(tank_capacity) + " " + to_string(t) + " " + to_string(total_energy_consumed);
        }
    private:
        string id;
};

class FertilizerDataStructure : public SensorDataStructure {
    public:
        bool activate = false;
        bool pumping = false;
        float nitrogen_concentration;
        float phosphorus_concentration;
        float potassium_concentration;
        float min_nitrogen_concentration;
        float min_phosphorus_concentration;
        float min_potassium_concentration;
        float fertilizer_amount_nitrogen_per_liter;
        float fertilizer_amount_phosphorus_per_liter;
        float fertilizer_amount_potassium_per_liter;
        int volume_per_minute;
        int command_volume = 0;
        float command_fertilizer_amount_nitrogen_per_liter = 0;
        float command_fertilizer_amount_phosphorus_per_liter = 0;
        float command_fertilizer_amount_potassium_per_liter = 0;
        int base_volume;
        int tank_capacity;
        int current_water_amount;
        time_t fertilizing_start_time;
        time_t fertilizing_end_time;
        FertilizerDataStructure(string id): SensorDataStructure() {
            type = "fertilizer";
            // Đọc từ file config dựa theo id của fertilizer
            // Mở file config
            this->id = id;
            string path = "config/fertilizer_config_" + id + ".txt";
            FILE* file = fopen(path.c_str(), "r");
            if (file == nullptr) {
                // Nếu không mở được file, đặt các giá trị mặc định
                min_nitrogen_concentration = 50.0;
                min_phosphorus_concentration = 30.0;
                min_potassium_concentration = 20.0;
                fertilizer_amount_nitrogen_per_liter = 5;
                fertilizer_amount_phosphorus_per_liter = 3;
                fertilizer_amount_potassium_per_liter = 3;
                fertilizing_start_time = 6 * 3600; // 6 AM
                fertilizing_end_time = 18 * 3600; // 6 PM
                base_volume = 20;
                volume_per_minute = 10;
                tank_capacity = 500;
                t = 1; // 1 minute
                // Lưu các giá trị mặc định vào file
                save_config();
            } else {
                // Đọc các thông số từ file
                fscanf(file, "%f %f %f %f %f %f %d %d %d %ld %ld %d", &min_nitrogen_concentration, &min_phosphorus_concentration, &min_potassium_concentration,
                       &fertilizer_amount_nitrogen_per_liter, &fertilizer_amount_phosphorus_per_liter, &fertilizer_amount_potassium_per_liter, &base_volume,
                       &volume_per_minute, &tank_capacity, (long*)&fertilizing_start_time, (long*)&fertilizing_end_time, &t);
                fclose(file);
            }
            current_water_amount = tank_capacity / 2;
            nitrogen_concentration = min_nitrogen_concentration;
            phosphorus_concentration = min_phosphorus_concentration;
            potassium_concentration = min_potassium_concentration;
        }
        int calculate_base_power() override {
            int power_consumption = 0;
            time_t current_time = time(nullptr) % 86400 + 7 * 3600; // adjust to GMT+7

            // timer
            if (timer_set && current_time < timer_time) {
                power_consumption += 2;
            }

            if (powered_on) {
                power_consumption += 5; // control circuit
            } else {
                return power_consumption;
            }

            // active fertilizing schedule
            if (current_time >= fertilizing_start_time && current_time <= fertilizing_end_time) {
                bool fertilizer_triggered = false;
                if (nitrogen_concentration < min_nitrogen_concentration && fertilizer_amount_nitrogen_per_liter > 0) {
                    power_consumption += 10;
                    fertilizer_triggered = true;
                }
                if (phosphorus_concentration < min_phosphorus_concentration && fertilizer_amount_phosphorus_per_liter > 0) {
                    power_consumption += 10;
                    fertilizer_triggered = true;
                }
                if (potassium_concentration < min_potassium_concentration && fertilizer_amount_potassium_per_liter > 0) {
                    power_consumption += 10;
                    fertilizer_triggered = true;
                }
                if (fertilizer_triggered) {
                    power_consumption += 2 * volume_per_minute;
                }
            }

            // pump to fill tank if low
            if (pumping) {
                power_consumption += 40;
            }

            return power_consumption;
        }

        int calculate_running_on_command_power() override {
            if (!powered_on) return 0;
            int power_consumption = 0;
            if (command_fertilizer_amount_nitrogen_per_liter > 0) {
                power_consumption += 10;
            }
            if (command_fertilizer_amount_phosphorus_per_liter > 0) {
                power_consumption += 10;
            }
            if (command_fertilizer_amount_potassium_per_liter > 0) {
                power_consumption += 10;
            }
            power_consumption += 2 * volume_per_minute;
            return power_consumption;
        }

        void save_config() {
            // Lưu các thông số vào file config
            string path = "config/fertilizer_config_" + id + ".txt";
            FILE* file = fopen(path.c_str(), "w");
            if (file != nullptr) {
            fprintf(file, "%f %f %f %f %f %f %d %d %d %ld %ld %d\n", min_nitrogen_concentration, min_phosphorus_concentration, min_potassium_concentration,
                fertilizer_amount_nitrogen_per_liter, fertilizer_amount_phosphorus_per_liter, fertilizer_amount_potassium_per_liter, base_volume,
                volume_per_minute, tank_capacity, (long)fertilizing_start_time, (long)fertilizing_end_time, t);
            fclose(file);
            }
        }
        void run(time_t start_time, time_t end_time) override {
            int duration_minutes = (end_time - start_time) / 60;

            int running_on_command_minutes = is_running_on_command ? command_volume / volume_per_minute : 0;
            if (running_on_command_minutes > duration_minutes) {
                running_on_command_minutes = duration_minutes;
                command_volume -= running_on_command_minutes * volume_per_minute;
            } else {
                command_volume = 0;
                is_running_on_command = false;
            }

            float energy_used = calculate_power_consumption(duration_minutes, running_on_command_minutes);
            total_energy_consumed += energy_used;

            // handle timer
            bool timer_triggered = false;
            time_t current_time = time(nullptr) % 86400 + 7 * 3600; // adjust to GMT+7
            if (timer_set && current_time >= timer_time) {
                powered_on = timer_set_to;
                timer_set = false;
                timer_triggered = true;
            }

            // update concentrations
            nitrogen_concentration -= duration_minutes * 0.1f; 
            phosphorus_concentration -= duration_minutes * 0.05f;
            potassium_concentration -= duration_minutes * 0.05f;

            // pump to fill tank
            if (current_water_amount < tank_capacity / 10) {
                pumping = true;
            } else if (current_water_amount >= tank_capacity) {
                pumping = false;
            }
            if (pumping) {
                int water_added = 2 * volume_per_minute * duration_minutes;
                current_water_amount += water_added;
            }
            
            if (nitrogen_concentration < min_nitrogen_concentration || phosphorus_concentration < min_phosphorus_concentration || potassium_concentration < min_potassium_concentration) {
                if (fertilizing_start_time <= fertilizing_end_time) {
                    // Khoảng thời gian không qua nửa đêm
                    if (current_time >= fertilizing_start_time && current_time <= fertilizing_end_time) {
                        activate = true;
                    }
                } else {
                    // Khoảng thời gian qua nửa đêm
                    if (current_time >= fertilizing_start_time || current_time <= fertilizing_end_time) {
                        activate = true;
                    }
                }
            }
            int auto_water_used = activate ? volume_per_minute * duration_minutes : 0;
            int command_water_used = volume_per_minute * running_on_command_minutes;
            int used = auto_water_used + command_water_used;
            if (used > current_water_amount) {
                used = current_water_amount;
                auto_water_used = used - command_water_used;
                if (auto_water_used < 0) {
                    auto_water_used = 0;
                    command_volume = command_water_used - used;
                    command_water_used = used;
                }
            }
            current_water_amount -= used;
            float total_added_n = (nitrogen_concentration < min_nitrogen_concentration ? fertilizer_amount_nitrogen_per_liter * auto_water_used : 0) + command_fertilizer_amount_nitrogen_per_liter * command_water_used;
            float total_added_p = (phosphorus_concentration < min_phosphorus_concentration ? fertilizer_amount_phosphorus_per_liter * auto_water_used : 0) + command_fertilizer_amount_phosphorus_per_liter * command_water_used;
            float total_added_k = (potassium_concentration < min_potassium_concentration ? fertilizer_amount_potassium_per_liter * auto_water_used : 0) + command_fertilizer_amount_potassium_per_liter * command_water_used;

            // Apply to concentrations (simple model): scale down total added to concentration units
            const float SCALE_DIV = 100.0f; 
            nitrogen_concentration += total_added_n / SCALE_DIV;
            phosphorus_concentration += total_added_p / SCALE_DIV;
            potassium_concentration += total_added_k / SCALE_DIV;

            if (current_water_amount < 0) current_water_amount = 0;
            if (current_water_amount > tank_capacity) current_water_amount = tank_capacity;
            if (nitrogen_concentration < 0) nitrogen_concentration = 0;
            if (phosphorus_concentration < 0) phosphorus_concentration = 0;
            if (potassium_concentration < 0) potassium_concentration = 0;
            cout << "Fertilizer " << id << " run from " << ctime(&start_time) << " to " << ctime(&end_time)
                 << " | Power: " << calculate_power() << " W"
                 << " | Energy used: " << energy_used << " kWh"
                 << " | N: " << nitrogen_concentration << "ppm, P: " << phosphorus_concentration << "ppm, K: " << potassium_concentration << "ppm"
                 << " | Water left: " << current_water_amount << " Liters" << endl;
            if (timer_triggered) {
                cout << "  (Timer triggered: Power set to " << (powered_on ? "ON" : "OFF") << ")" << endl;
            }
        }

        string toString()
        {
            return to_string(fertilizer_amount_nitrogen_per_liter) + " " + to_string(nitrogen_concentration) + " " + to_string(min_nitrogen_concentration) + " " + to_string(fertilizer_amount_phosphorus_per_liter) + " " + to_string(phosphorus_concentration) + " " + to_string(min_phosphorus_concentration) + " " + to_string(fertilizer_amount_potassium_per_liter) + " " + to_string(potassium_concentration) + " " + to_string(min_potassium_concentration)
                   + " " + to_string(fertilizing_start_time) + " " + to_string(fertilizing_end_time) + " " + to_string(t) + " " + to_string(total_energy_consumed);
        }
    private:
        string id;
};

class LightingDataStructure : public SensorDataStructure {
    public:
        int lightPower;
        int command_lightPower = 0;
        int command_duration_minutes = 0;
        time_t lighting_start_time;
        time_t lighting_end_time;
        LightingDataStructure(string id): SensorDataStructure() {
            type = "lighting";
            // Đọc từ file config dựa theo id của lighting
            // Mở file config
            this->id = id;
            string path = "config/lighting_config_" + id + ".txt";
            FILE* file = fopen(path.c_str(), "r");
            if (file == nullptr) {
                // Nếu không mở được file, đặt các giá trị mặc định
                lightPower = 100; // Watts
                lighting_start_time = 18 * 3600; // 6 PM
                lighting_end_time = 6 * 3600; // 6 AM
                t = 1; // 1 minute
                // Lưu các giá trị mặc định vào file
                save_config();
            } else {
                // Đọc các thông số từ file
                fscanf(file, "%d %ld %ld %d", &lightPower, &lighting_start_time, &lighting_end_time, &t);
                fclose(file);
            }
        }
        void save_config() {
            // Lưu các thông số vào file config
            string path = "config/lighting_config_" + id + ".txt";
            FILE* file = fopen(path.c_str(), "w");
            if (file != nullptr) {
                fprintf(file, "%d %ld %ld %d\n", lightPower, lighting_start_time, lighting_end_time, t);
                fclose(file);
            }
        }
        int calculate_base_power() override {
            int power = 0;
            time_t current_time = time(nullptr) % 86400 + 7 * 3600; // adjust to GMT+7
            if (timer_set && current_time < timer_time) power += 2;
            if (powered_on) power += 3; else return power;
            if (lighting_start_time < lighting_end_time) {
                // Khoảng thời gian không qua nửa đêm
                if (current_time >= lighting_start_time && current_time <= lighting_end_time) {
                    power += lightPower;
                }
            } else {
                // Khoảng thời gian qua nửa đêm
                if (current_time >= lighting_start_time || current_time <= lighting_end_time) {
                    power += lightPower;
                }
            }
            return power;
        }

        int calculate_running_on_command_power() override {
            return is_running_on_command ? command_lightPower - lightPower : 0;
        }

        void run(time_t start_time, time_t end_time) override {
            int duration_minutes = (end_time - start_time) / 60;
            
            int running_on_command_minutes = is_running_on_command ? command_duration_minutes : 0;
            if (running_on_command_minutes > duration_minutes) {
                running_on_command_minutes = duration_minutes;
                command_duration_minutes -= running_on_command_minutes;
            } else {
                command_duration_minutes = 0;
                is_running_on_command = false;
            }
            float energy_used = calculate_power_consumption(duration_minutes, running_on_command_minutes);
            total_energy_consumed += energy_used;

            time_t current_time = time(nullptr) % 86400 + 7 * 3600; // adjust to GMT+7
            bool timer_triggered = false;
            if (timer_set && current_time >= timer_time) {
                powered_on = timer_set_to;
                timer_set = false;
                timer_triggered = true;
            }
            
            cout << "Lighting " << id << " run from " << ctime(&start_time) << " to " << ctime(&end_time)
                 << " | Power: " << calculate_power() << " W"
                 << " | Energy used: " << energy_used << " kWh" << endl;
            if (timer_triggered) {
                cout << "  (Timer triggered: Power set to " << (powered_on ? "ON" : "OFF") << ")" << endl;
            }
        }

        string toString()
        {
            return to_string(lightPower) + " " + to_string(lighting_start_time) + " " + to_string(lighting_end_time) + " " + to_string(t) + " " + to_string(total_energy_consumed);
        }
    private:
        string id;
};
#endif // SENSORSERVER_SENSORDATASTRUCTURE_HPP