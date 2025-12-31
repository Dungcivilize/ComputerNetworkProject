#include <string>

void log(std::string id, std::string message) {
    FILE *file = fopen(("sensor_server_logs/" + id + ".log").c_str(), "a");
    if (file) {
        // Log cả thời gian và thông điệp
        fprintf(file, "[%s] %s\n", __TIME__, message.c_str());
        fclose(file);
    } else {
        perror("Failed to open log file");
    }
}