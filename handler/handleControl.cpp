#include "../common/index.hpp"

void power_control(Device* selected_device)
{
    cout << "Enter power state (1 for ON, 0 for OFF): ";
    int state;
    cin >> state;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (state != 0 && state != 1) {
        cout << " Invalid power state." << endl;
        return;
    }
    call_api(selected_device->sockfd, to_string(3) + " " + selected_device->info.token + " " + to_string(state));
}

void take_control(Device* selected_device)
{
    if (!selected_device)
        return;
    if (selected_device->info.type == SPRINKLER) {
        cout << "Enter watering amount in liters (input 0 for default, -1 to cancel): ";
        int amount;
        cin >> amount;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (amount == -1) {
            cout << "Watering cancelled." << endl;
            return;
        }
        call_api(selected_device->sockfd, to_string(3) + " " + selected_device->info.token + " " + to_string(amount));
    }
    else if (selected_device->info.type == FERTILIZER) {
        // Nhập nồng độ phân đạm
        cout << "Enter the Concentration of nitrogen in mg/L (input 0 for default, -1 to cancel): ";
        int cn;
        cin >> cn;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (cn == -1) {
            cout << "Fertilizing cancelled." << endl;
            return;
        }
        cout << "Enter the Concentration of phosphorus in mg/L (input 0 for default, -1 to cancel): ";
        int cp;
        cin >> cp;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (cp == -1) {
            cout << "Fertilizing cancelled." << endl;
            return;
        }
        cout << "Enter the Concentration of potassium in mg/L (input 0 for default, -1 to cancel): ";
        int ck;
        cin >> ck;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (ck == -1) {
            cout << "Fertilizing cancelled." << endl;
            return;
        }
        cout << "Enter the volume to fertilize in liters (input 0 for default, -1 to cancel): ";
        int volume;
        cin >> volume;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (volume == -1) {
            cout << "Fertilizing cancelled." << endl;
            return;
        }
        call_api(selected_device->sockfd, to_string(3) + " " + selected_device->info.token + " " + to_string(cn) + " " + to_string(cp) + " " + to_string(ck) + " " + to_string(volume));
    }
    else if (selected_device->info.type == LIGHTING) {
        cout << "Enter lighting duration in minutes (input -1 to cancel): ";
        int duration;
        cin >> duration;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (duration == -1) {
            cout << "Lighting cancelled." << endl;
            return;
        }
        cout << "Enter lighting power in Watts (input 0 for default, -1 to cancel): ";
        int power;
        cin >> power;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (power == -1) {
            cout << "Lighting cancelled." << endl;
            return;
        }
        call_api(selected_device->sockfd, to_string(3) + " " + selected_device->info.token + " " + to_string(duration) + " " + to_string(power));
    }
}

void set_timer(Device* selected_device)
{
    cout << "Enter state (1 for ON, 0 for OFF, -1 to cancel): ";
    int state;
    cin >> state;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (state == -1)
    {
        cout << "Timer setting cancelled." << endl;
        return;
    }

    cout << "Enter time to perform action (minutes from now, -1 to cancel): ";
    int minutes;
    cin >> minutes;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (minutes == -1)
    {
        cout << "Timer setting cancelled." << endl;
        return;
    }
    call_api(selected_device->sockfd, to_string(3) + " " + selected_device->info.token + " 3 " + to_string(state) + " " + to_string(minutes));
}   

void cancel_control(Device* selected_device)
{
    call_api(selected_device->sockfd, to_string(3) + " " + selected_device->info.token + " 4");
}

void handleControl(std::vector<Device*>& devices)
{
    Device* selected_device = listDeviceToSelect(devices);
    if (!selected_device)
        return;
    if (selected_device->info.token.empty()) {
        cout << " Device not connected properly." << endl;
        return;
    }
    cout << "Available control options:" << endl;
    cout << "1. Power Control" << endl;
    switch (selected_device->info.type) {
        case SPRINKLER:
            cout << "2. Watering Control" << endl;
            break;
        case FERTILIZER:
            cout << "2. Fertilizing Control" << endl;
            break;
        case LIGHTING:
            cout << "2. Lighting Control" << endl;
            break;
        default:
            cout << " Unknown device type." << endl;
            return;
    }
    cout << "3. Set Timer" << endl;
    cout << "4. Cancel Control" << endl;
    cout << "Select an option (1-4): ";
    int option;
    cin >> option;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    switch (option) {
        case 1:
            power_control(selected_device);
            break;
        case 2:
            take_control(selected_device);
            break;
        case 3:
            set_timer(selected_device);
            break;
        case 4:
            cancel_control(selected_device);
            break;
        default:
            cout << " Invalid option selected." << endl;
            break;
    }
}