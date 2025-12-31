#include "handleChangeInfo.hpp"

void handleChangeInfo(std::vector<Device*>& devices) {
    Device* selected_device = listDeviceToSelect(devices);
    if (!selected_device)
        return;
    cout << "Basic params: " << endl;
    cout << "  1. Sensor refresh time: (minutes) " << endl;
    cout << "  2. Start Time (HH:MM:SS) " << endl;
    cout << "  3. End Time (HH:MM:SS) " << endl;
    cout << "Specific params: " << endl;
    if (selected_device->info.type == SPRINKLER) {
        cout << "  4. Water Volume per Minute (Liters) " << endl;
        cout << "  5. Min Humidity (%) " << endl;
        cout << "  6. Max Humidity (%) " << endl;
    } else if (selected_device->info.type == FERTILIZER) {
        cout << "  4. Fertilizer Amount per Liter - Nitrogen (mg/L) " << endl;
        cout << "  5. Min Nitrogen Concentration (ppm) " << endl;
        cout << "  6. Fertilizer Amount per Liter - Phosphorus (mg/L) " << endl;
        cout << "  7. Min Phosphorus Concentration (ppm) " << endl;
        cout << "  8. Fertilizer Amount per Liter - Potassium (mg/L) " << endl;
        cout << "  9. Min Potassium Concentration (ppm) " << endl;
        cout << " 10. Water volume per minute (Liters) " << endl;
    } else if (selected_device->info.type == LIGHTING) {
        cout << "  4. Light Power (Watts) " << endl;
    }
    cout << "Enter parameter number to change (-1 to cancel): ";
    int param_number;
    cin >> param_number;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (param_number == -1)
        return;
    cout << "Enter new value: ";
    string new_value;
    getline(cin, new_value);
    // Nếu là thời gian, kiểm tra định dạng
    time_t temp_time;
    if (param_number == 2 || param_number == 3) {
        temp_time = convert_string_only_time_to_time_t(new_value);
        if (temp_time == (time_t)-1) {
            cout << " Invalid time format." << endl;
            return; 
        }
        new_value = to_string(temp_time);
    }
    call_api(selected_device->sockfd, to_string(6) + " " + selected_device->info.token + " " + to_string(param_number) + " " + new_value);
}