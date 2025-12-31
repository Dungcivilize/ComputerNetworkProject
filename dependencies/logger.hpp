#include <string>
#include <sys/stat.h>
#include "../common/register.hpp"

void log(std::string message) {
    int app_id = check();
    // Tạo thư mục nếu chưa tồn tại
    std::string log_dir = "client_logs/";
    system("mkdir -p client_logs");

    FILE *file = fopen(("client_logs/" + std::to_string(app_id) + ".log").c_str(), "a");
    if (file) {
        // Log both time and message
        fprintf(file, "[%s] %s\n", __TIME__, message.c_str());
        fclose(file);
    } else {
        perror("Failed to open log file");
    }
}