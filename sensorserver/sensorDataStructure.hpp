#ifndef SENSORSERVER_SENSORDATASTRUCTURE_HPP
#define SENSORSERVER_SENSORDATASTRUCTURE_HPP

#include <string>
#include <ctime>
#include <iostream>
using namespace std;

class SensorDataStructure {
    public:
        bool powered_on;
        bool timer_set_to;
        time_t timer_time;
        string current_action;
        int t;
        SensorDataStructure() : powered_on(false), timer_set_to(false), timer_time(0), current_action("NONE"), t(5) {}
        int calculate_power() {
            int power_consumption = 0; // đơn vị: Watt

            if (powered_on) {
                power_consumption += 3; // mạch điều khiển
            } else {
                return 0;
            }

            time_t current_time = time(nullptr);

            // Nếu đang chạy chế độ hẹn giờ
            if (current_action == "TIMER" && current_time < timer_time) {
                power_consumption += 1; // bộ hẹn giờ
            }

            return power_consumption; // tổng công suất tức thời (W)
        }
        float calculate_power_consumption(int duration_minutes) {
            int power_consumption = calculate_power(); // công suất tức thời (W)
            return power_consumption * duration_minutes / 60 / 1000.0; // tổng năng lượng tiêu thụ (kWh)
        }
        void save_config() {
            // Base class has no config to save
        }
        string toString()
        {
            return "";
        }
};

class SprinklerDataStructure : public SensorDataStructure {
    public:
        float humidity;
        float min_humidity;
        float max_humidity;
        int volume_per_minute;
        int volume_per_humidity_percent;
        int base_volume;
        int tank_capacity;
        int current_water_amount;
        time_t watering_start_time;
        time_t watering_end_time;
        SprinklerDataStructure(string id) : SensorDataStructure() {
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
                // Lưu các giá trị mặc định vào file
                save_config();
            } else {
                // Đọc các thông số từ file
                fscanf(file, "%f %f %d %d %d %d", &min_humidity, &max_humidity, &volume_per_minute, &volume_per_humidity_percent, &base_volume, &tank_capacity);
                fclose(file);
            }
        }
        int calculate_power() {
            int power_consumption = 0; // đơn vị: Watt

            if (powered_on) {
                power_consumption += 5; // mạch điều khiển
            } else {
                return 0;
            }

            time_t current_time = time(nullptr);

            // Nếu đang tưới trong khoảng thời gian định sẵn
            if (current_action == "NONE" && current_time >= watering_start_time &&
                current_time <= watering_end_time) {
                power_consumption += 20; // van điện từ
                // Nếu độ ẩm hiện tại thấp hơn mức tối thiểu
                if (humidity < min_humidity) {
                    power_consumption += 50;
                }
                // Nếu độ ẩm hiện tại thấp hơn mức tối đa 
                if (humidity <= max_humidity) {
                    // Dựa trên lượng nước tưới mỗi phút để tính công suất
                    power_consumption += 2 * volume_per_minute;
                }
            }

            // Nếu trạng thái hiện tại là đang tưới nước (bơm hoạt động)
            if (current_action == "WATERING") {
                power_consumption += 2 * volume_per_minute;
            }

            // Nếu bơm đang nạp nước vào bể
            if (current_water_amount < tank_capacity / 10) {
                // Công suất máy bơm vào bể cố định ở mức 50W
                power_consumption += 50; // bơm nước vào bể 
            }

            // Nếu đang chạy chế độ hẹn giờ
            if (current_action == "TIMER") {
                power_consumption += 2; // bộ hẹn giờ
            }

            return power_consumption; // tổng công suất tức thời (W)
        }

        void save_config() {
            // Lưu các thông số vào file config
            string path = "config/sprinkler_config_" + id + ".txt";
            FILE* file = fopen(path.c_str(), "w");
            if (file != nullptr) {
                fprintf(file, "%f %f %d %d %d %d\n", min_humidity, max_humidity, volume_per_minute, volume_per_humidity_percent, base_volume, tank_capacity);
                fclose(file);
            }
        }

        string toString()
        {
            return to_string(min_humidity) + " " + to_string(max_humidity) + " " + to_string(watering_start_time) + " " + to_string(watering_end_time);
        }
    private:
        string id;
};

class fertilizerDataStructure : public SensorDataStructure {
    public:
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
        int base_volume;
        int tank_capacity;
        int current_warter_amount;
        time_t fertilizing_start_time;
        time_t fertilizing_end_time;
        fertilizerDataStructure(string id): SensorDataStructure() {
            // Đọc từ file config dựa theo id của fertilizer
            // Mở file config
            this->id = id;
            string path = "config/fertilizer_config_" + id + ".txt";
            FILE* file = fopen(path.c_str(), "r");
            if (file == nullptr) {
                // Nếu không mở được file, đặt các giá trị mặc định
                min_nitrogen_concentration = 10.0;
                min_phosphorus_concentration = 5.0;
                min_potassium_concentration = 5.0;
                fertilizer_amount_nitrogen_per_liter = 50.0;
                fertilizer_amount_phosphorus_per_liter = 30.0;
                fertilizer_amount_potassium_per_liter = 30.0;
                volume_per_minute = 10;
                tank_capacity = 500;
                // Lưu các giá trị mặc định vào file
                save_config();
            } else {
                // Đọc các thông số từ file
                fscanf(file, "%f %f %f %f %f %f %d %d", &min_nitrogen_concentration, &min_phosphorus_concentration, &min_potassium_concentration,
                       &fertilizer_amount_nitrogen_per_liter, &fertilizer_amount_phosphorus_per_liter, &fertilizer_amount_potassium_per_liter,
                       &volume_per_minute, &tank_capacity);
                fclose(file);
            }
        }
        void save_config() {
            // Lưu các thông số vào file config
            string path = "config/fertilizer_config_" + id + ".txt";
            FILE* file = fopen(path.c_str(), "w");
            if (file != nullptr) {
                fprintf(file, "%f %f %f %f %f %f %d %d\n", min_nitrogen_concentration, min_phosphorus_concentration, min_potassium_concentration,
                        fertilizer_amount_nitrogen_per_liter, fertilizer_amount_phosphorus_per_liter, fertilizer_amount_potassium_per_liter,
                        volume_per_minute, tank_capacity);
                fclose(file);
            }
        }
        string toString()
        {
            return to_string(min_nitrogen_concentration) + " " + to_string(min_phosphorus_concentration) + " " + to_string(min_potassium_concentration)
                   + " " + to_string(fertilizing_start_time) + " " + to_string(fertilizing_end_time);
        }
    private:
        string id;
};

class lightingDataStructure : public SensorDataStructure {
    public:
        int lightPower;
        time_t lighting_start_time;
        time_t lighting_end_time;
        lightingDataStructure(string id): SensorDataStructure() {
            // Đọc từ file config dựa theo id của lighting
            // Mở file config
            this->id = id;
            string path = "config/lighting_config_" + id + ".txt";
            FILE* file = fopen(path.c_str(), "r");
            if (file == nullptr) {
                // Nếu không mở được file, đặt các giá trị mặc định
                lightPower = 100; // Watts
                // Lưu các giá trị mặc định vào file
                save_config();
            } else {
                // Đọc các thông số từ file
                fscanf(file, "%d", &lightPower);
                fclose(file);
            }
        }
        void save_config() {
            // Lưu các thông số vào file config
            string path = "config/lighting_config_" + id + ".txt";
            FILE* file = fopen(path.c_str(), "w");
            if (file != nullptr) {
                fprintf(file, "%d\n", lightPower);
                fclose(file);
            }
        }
        string toString()
        {
            return to_string(lightPower) + " " + to_string(lighting_start_time) + " " + to_string(lighting_end_time);
        }
    private:
        string id;
};
#endif // SENSORSERVER_SENSORDATASTRUCTURE_HPP