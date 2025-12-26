#include "framework.hpp"
#include "device.hpp"
#include "scan.hpp"
#include "utils.hpp"
#include "connect.hpp"
#include "changepassword.hpp"

static void execute_scan(vector<DeviceInfo>& available, const string& id)
{
    if (!scan(available, id))
        cerr << "An error occured while scanning for devices" << endl;
    else
    {
        if (available.empty())
            cout << "No device found" << endl;
        else
        {
            cout << "Device found:" << endl;
            for (auto& dev : available)
                cout << dev.type + " " + dev.id + " " + dev.name << endl;
        }
    }
}

static void execute_connect(vector<DeviceInfo>& available, const string& device_id, const string& app_id, const string& password, vector<Device>& connected)
{
    size_t index = -1;
    for (size_t idx = 0; idx < available.size(); idx++)
        if (available[idx].id == device_id)
        {
            index = idx;
            break;
        }
    if (index == -1)
    {
        cerr << "Unknown device ID. Please use SCAN to check for this ID again" << endl;
        return;
    }

    int code = 0;
    string resp;
    if (connection(available[index], app_id, password, resp, code, connected))
        cout << "Connection successful with code " + to_string(code) << endl;
    else
        cerr << "Err " + to_string(code) + ": " + resp << endl; 
}

static void execute_change_password(vector<Device>& connected, const string& device_id, const string& current_pass, const string& new_pass)
{
    size_t index = -1;
    for (size_t idx = 0; idx < connected.size(); idx++)
        if (connected[idx].id == device_id)
        {
            index = idx;
            break;
        }
    if (index == -1)
    {
        cerr << "You have not connected to device with this ID yet" << endl;
        return;
    }

    int code = 0;
    string resp;
    if (change_password(connected[index], current_pass, new_pass, resp, code))
        cout << "Change password successful" << endl;
    else
        cerr << "Err " + to_string(code) + ": " + resp << endl;
}

int main(int argc, char* argv[])
{
    string id = randstr(ID_SIZE);
    cout << "Application running. ID: " + id << endl;
    
    string cmdline;
    vector<DeviceInfo> available;
    vector<Device> connected;
    while (true)
    {
        getline(cin, cmdline);
        stringstream ss(cmdline);

        string cmd;
        ss >> cmd;

        if (cmd == "SCAN")
            execute_scan(available, id);
        else if (cmd == "CONNECT")
        {
            string device_id, password;
            ss >> device_id >> password;
            execute_connect(available, device_id, id, password, connected);
        }
        else if (cmd == "CHANGE_PW")
        {
            string device_id, current_pass, new_pass;
            ss >> device_id >> current_pass >> new_pass;
            execute_change_password(connected, device_id, current_pass, new_pass);
        }
        else
            cerr << "Unknown command" << endl;
    }
    return 0;
}