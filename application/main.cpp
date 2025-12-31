#include "framework.hpp"
#include "device.hpp"
#include "scan.hpp"
#include "utils.hpp"
#include "connect.hpp"
#include "changepassword.hpp"
#include "config.hpp"
#include "power.hpp"
#include "light.hpp"
#include "schedule.hpp"
#include "water.hpp"
#include "fertilize.hpp"

void display_info(vector<Device>& devices, const string& id)
{
    ssize_t index = find_by_id(devices, id);
    if (index >= 0)
        cout << "Device info:\n" + devices[index].display() << endl;
    else
        cerr << "Error: No such device is connected." << endl;
}

int main(int argc, char* argv[])
{
    string id = randstr(ID_SIZE);
    cout << "Application running. ID: " + id << endl;
    cout << "Type \"help all\" to see all action." << endl;
    
    string cmdline;
    vector<DeviceInfo> available;
    vector<Device> connected;
    while (true)
    {
        cout << "> ";   
        getline(cin, cmdline);
        stringstream ss(cmdline);

        string cmd;
        ss >> cmd;

        if (cmd == "scan")
            scan(available, id);
        else if (cmd == "connect")
        {
            string device_id, password;
            ss >> device_id >> password;
            execute_connect(available, device_id, id, password, connected);
        }
        else if (cmd == "changepassword")
        {
            string device_id, current_pass, new_pass;
            ss >> device_id >> current_pass >> new_pass;
            execute_change_password(connected, device_id, current_pass, new_pass);
        }
        else if (cmd == "config")
        {
            string device_id, param;
            int value;
            ss >> device_id >> param >> value;
            execute_config(connected, device_id, param, value);
        }
        else if (cmd == "quit")
        {
            cout << "Program terminated." << endl;
            break;
        }
        else if (cmd == "display")
        {
            string device_id;
            ss >> device_id;
            display_info(connected, device_id);
        }
        else if (cmd == "power")
        {
            string device_id, mode;
            ss >> device_id >> mode;
            execute_power(connected, device_id, mode);
        }
        else if (cmd == "light")
        {
            string device_id;
            int p, s;
            ss >> device_id >> p >> s;
            execute_light(connected, device_id, p, s);
        }
        else if (cmd == "water")
        {
            string device_id;
            int v;
            ss >> device_id >> v;
            execute_water(connected, device_id, v);
        }
        else if (cmd == "fertilize")
        {
            string device_id;
            int c, v;
            ss >> device_id >> c >> v;
            execute_fertilize(connected, device_id, c, v);
        }
        else if (cmd == "schedule")
        {
            string device_id, scope, timestamp;
            ss >> device_id >> scope >> timestamp;
            execute_schedule(connected, device_id, scope, timestamp);
        }
        else
            cerr << "Unknown action. Type \"help all\" to see all actions." << endl;
    }
    return 0;
}