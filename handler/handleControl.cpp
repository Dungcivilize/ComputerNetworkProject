#include "../common/index.hpp"

void power_control(vector<Device*>& devices)
{
    selected_device = listDeviceToSelect(devices);
    if (!selected_device)
        continue;
    if (selected_device->info.token.empty()) {
        cout << " Device not connected properly." << endl;
        continue;
    }
    cout << "Enter power state (0 for ON, 1 for OFF): ";
    int state;
    cin >> state;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (state != 0 && state != 1) {
        cout << " Invalid power state." << endl;
        continue;
    }
    call_api(selected_device->sockfd, to_string(3) + " " + selected_device->info.token + " " + to_string(state));
}

void take_control(vector<Device*>& devices)
{
    selected_device = listDeviceToSelect(devices);
    if (!selected_device)
        continue;
    if (selected_device->info.type == SPRINKLER) {
        cout << "Enter watering amount in liters (input 0 for default, -1 to cancel): ";
        int amount;
        cin >> amount;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (amount == -1) {
            cout << "Watering cancelled." << endl;
            continue;
        }
        call_api(selected_device->sockfd, to_string(4) + " " + selected_device->info.token + " " + to_string(amount));
    }
    else if (selected_device->info.type == FERTILIZER) {
        // Nhập nồng độ phân đạm
        cout << "Enter the Concentration of nitrogen in mg/L (input 0 for default, -1 to cancel): ";
        int cn;
        cin >> cn;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (cn == -1) {
            cout << "Fertilizing cancelled." << endl;
            continue;
        }
        cout << "Enter the Concentration of phosphorus in mg/L (input 0 for default, -1 to cancel): ";
        int cp;
        cin >> cp;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (cp == -1) {
            cout << "Fertilizing cancelled." << endl;
            continue;
        }
        cout << "Enter the Concentration of potassium in mg/L (input 0 for default, -1 to cancel): ";
        int ck;
        cin >> ck;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (ck == -1) {
            cout << "Fertilizing cancelled." << endl;
            continue;
        }
        cout << "Enter the volume to fertilize in liters (input 0 for default, -1 to cancel): ";
        int volume;
        cin >> volume;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (volume == -1) {
            cout << "Fertilizing cancelled." << endl;
            continue;
        }
        call_api(selected_device->sockfd, to_string(4) + " " + selected_device->info.token + " " + to_string(cn) + " " + to_string(cp) + " " + to_string(ck) + " " + to_string(volume));
    }
    else if (selected_device->info.type == LIGHTING) {
        cout << "Enter lighting duration in minutes (input -1 to cancel): ";
        int duration;
        cin >> duration;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (duration == -1) {
            cout << "Lighting cancelled." << endl;
            continue;
        }
        cout << "Enter lighting power in Watts (input 0 for default, -1 to cancel): ";
        int power;
        cin >> power;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (power == -1) {
            cout << "Lighting cancelled." << endl;
            continue;
        }
        call_api(selected_device->sockfd, to_string(4) + " " + selected_device->info.token + " " + to_string(duration) + " " + to_string(power));
    }
}

void set_timer(vector<Device*>& devices)
{
    selected_device = listDeviceToSelect(devices);
    if (!selected_device)
        continue;
    cout << "Enter state (0 to ON, 1 to OFF, -1 to cancel): ";
    int state;
    cin >> state;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (state == -1)
    {
        cout << "Timer setting cancelled." << endl;
        continue;
    }

    cout << "Enter time to perform action (minutes from now, -1 to cancel): ";
    int minutes;
    cin >> minutes;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (minutes == -1)
    {
        cout << "Timer setting cancelled." << endl;
        continue;
    }
    call_api(selected_device->sockfd, to_string(5) + " " + selected_device->info.token + " " + to_string(state) + " " + to_string(minutes));
}   

void cancel_control(vector<Device*>& devices)
{
    selected_device = listDeviceToSelect(devices);
    if (!selected_device)
        continue;
    call_api(selected_device->sockfd, to_string(6) + " " + selected_device->info.token);
}