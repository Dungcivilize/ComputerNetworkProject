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

void sensor_help()
{
    cout << "1. view_info : Viewing device's ID and type" << endl;
    cout << "2. query <scope>: Querying device's info" << endl;
    cout << "Scope values: " << endl;
    cout << "STATE : Querying device's current active state" << endl;
    cout << "PARAM : Querying device's working parameters" << endl;
    cout << "SCHEDULE : Querying device's working schedule" << endl;
    cout << "ALL : Querying all device's info" << endl;
    cout << "3. power <value> : Turn on/off device. Value must either be ON or OFF" << endl;
    cout << "4. changepassword <current_pass> <new_pass> : Change device's password" << endl;
    cout << "5. config <param> <value> : Configurate device's parameter" << endl;
    cout << "Available param for sensor:" << endl;
    cout << "T: Sensor record stats every T minutes" << endl;
    cout << "6. unselect : Quit device controlling" << endl;
}

void sensor_interface(Device& device)
{
    while (true)
    {
        cout << "> ";
        string cmdline;  
        getline(cin, cmdline);
        stringstream ss(cmdline);

        string cmd;
        ss >> cmd;

        if (cmd == "view_info")
            cout << "ID: " + device.id << "\nType: " + device.type << endl;
        else if (cmd == "query")
        {
            string scope;
            ss >> scope;
            execute_query(device, scope);
        }
        else if (cmd == "power")
        {
            string mode;
            ss >> mode;
            execute_power(device, mode);
        }
        else if (cmd == "changepassword")
        {
            string current_pass, new_pass;
            ss >> current_pass >> new_pass;
            execute_change_password(device, current_pass, new_pass);
        }
        else if (cmd == "config")
        {
            string param;
            int value;
            ss >> param >> value;
            execute_config(device, param, value);
        }
        else if (cmd == "unselect")
            break;
        else if (cmd == "help")
            sensor_help();
        else
            cerr << "Unknown action. Type \"help\" to see all actions." << endl;
    }
}

void light_help()
{
    cout << "1. view_info : Viewing device's ID and type" << endl;
    cout << "2. query <scope>: Querying device's info" << endl;
    cout << "Scope values: " << endl;
    cout << "- STATE : Querying device's current active state" << endl;
    cout << "- PARAM : Querying device's working parameters" << endl;
    cout << "- SCHEDULE : Querying device's working schedule" << endl;
    cout << "- ALL : Querying all device's info" << endl;
    cout << "3. power <value> : Turn on/off device. Value must either be ON or OFF" << endl;
    cout << "4. changepassword <current_pass> <new_pass> : Change device's password" << endl;
    cout << "5. config <param> <value> : Configurate device's parameter" << endl;
    cout << "Available param for light:" << endl;
    cout << "- P: Light's power" << endl;
    cout << "- S: Lighting duration in second" << endl;
    cout << "6. light <P> <S>: Turn on the light now with power P and duration S." << endl;
    cout << "7. schedule <mode> <datetime> : Add/remove a timestamp for daily lighting. Mode must either be ADD or REMOVE. Date time must have the format HH:MM:SS" << endl;
    cout << "8. unselect : Quit device controlling" << endl;
}

void light_interface(Device& device)
{
    while (true)
    {
        cout << "> ";
        string cmdline;  
        getline(cin, cmdline);
        stringstream ss(cmdline);

        string cmd;
        ss >> cmd;

        if (cmd == "view_info")
            cout << "ID: " + device.id << "\nType: " + device.type << endl;
        else if (cmd == "query")
        {
            string scope;
            ss >> scope;
            execute_query(device, scope);
        }
        else if (cmd == "power")
        {
            string mode;
            ss >> mode;
            execute_power(device, mode);
        }
        else if (cmd == "changepassword")
        {
            string current_pass, new_pass;
            ss >> current_pass >> new_pass;
            execute_change_password(device, current_pass, new_pass);
        }
        else if (cmd == "config")
        {
            string param;
            int value;
            ss >> param >> value;
            execute_config(device, param, value);
        }
        else if (cmd == "light")
        {
            int p, s;
            ss >> p >> s;
            execute_light(device, p, s);
        }
        else if (cmd == "schedule")
        {
            string scope, time;
            ss >> scope >> time;
            execute_schedule(device, scope, time);
        }
        else if (cmd == "unselect")
            break;
        else if (cmd == "help")
            light_help();
        else
            cerr << "Unknown action. Type \"help\" to see all actions." << endl;
    }
}

void sprinkler_help()
{
    cout << "1. view_info : Viewing device's ID and type" << endl;
    cout << "2. query <scope>: Querying device's info" << endl;
    cout << "Scope values: " << endl;
    cout << "- STATE : Querying device's current active state" << endl;
    cout << "- PARAM : Querying device's working parameters" << endl;
    cout << "- SCHEDULE : Querying device's working schedule" << endl;
    cout << "- ALL : Querying all device's info" << endl;
    cout << "3. power <value> : Turn on/off device. Value must either be ON or OFF" << endl;
    cout << "4. changepassword <current_pass> <new_pass> : Change device's password" << endl;
    cout << "5. config <param> <value> : Configurate device's parameter" << endl;
    cout << "Available param for sprinkler:" << endl;
    cout << "- Hmin: Minimum humidity for immediate watering" << endl;
    cout << "- Hmax: Maximum humidity, if current humidity lower than Hmax then automatically watering according to schedule" << endl;
    cout << "- V: Amount of water for sprinkling" << endl;
    cout << "6. water <V>: watering now with V litters" << endl;
    cout << "7. schedule <mode> <datetime> : Add/remove a timestamp for daily watering. Mode must either be ADD or REMOVE. Date time must have the format HH:MM:SS" << endl;
    cout << "8. unselect : Quit device controlling" << endl;
}

void sprinkler_interface(Device& device)
{
    while (true)
    {
        cout << "> ";
        string cmdline;  
        getline(cin, cmdline);
        stringstream ss(cmdline);

        string cmd;
        ss >> cmd;

        if (cmd == "view_info")
            cout << "ID: " + device.id << "\nType: " + device.type << endl;
        else if (cmd == "query")
        {
            string scope;
            ss >> scope;
            execute_query(device, scope);
        }
        else if (cmd == "power")
        {
            string mode;
            ss >> mode;
            execute_power(device, mode);
        }
        else if (cmd == "changepassword")
        {
            string current_pass, new_pass;
            ss >> current_pass >> new_pass;
            execute_change_password(device, current_pass, new_pass);
        }
        else if (cmd == "config")
        {
            string param;
            int value;
            ss >> param >> value;
            execute_config(device, param, value);
        }
        else if (cmd == "water")
        {
            int v;
            ss >> v;
            execute_water(device, v);
        }
        else if (cmd == "schedule")
        {
            string scope, time;
            ss >> scope >> time;
            execute_schedule(device, scope, time);
        }
        else if (cmd == "unselect")
            break;
        else if (cmd == "help")
            sprinkler_help();
        else
            cerr << "Unknown action. Type \"help\" to see all actions." << endl;
    }
}

void fertilizer_help()
{
    cout << "1. view_info : Viewing device's ID and type" << endl;
    cout << "2. query <scope>: Querying device's info" << endl;
    cout << "Scope values: " << endl;
    cout << "- STATE : Querying device's current active state" << endl;
    cout << "- PARAM : Querying device's working parameters" << endl;
    cout << "- SCHEDULE : Querying device's working schedule" << endl;
    cout << "- ALL : Querying all device's info" << endl;
    cout << "3. power <value> : Turn on/off device. Value must either be ON or OFF" << endl;
    cout << "4. changepassword <current_pass> <new_pass> : Change device's password" << endl;
    cout << "5. config <param> <value> : Configurate device's parameter" << endl;
    cout << "Available param for sprinkler:" << endl;
    cout << "- C: Fertilizer concentration" << endl;
    cout << "- V: Amount of water for mixing fertilizer" << endl;
    cout << "- Nmin: Minimum nitrogen concentration for automatically fertilizing" << endl;
    cout << "- Pmin: Minimum phosphorous concentration for automatically fertilizing" << endl;
    cout << "- Kmin: Minimum kali concentration for automatically fertilizing" << endl;
    cout << "6. fertilize <C> <K> : Fertilizing now with concentration C(mg/L) and V(L) of waters" << endl;
    cout << "7. unselect : Quit device controlling" << endl;
}

void fertilizer_interface(Device& device)
{
    while (true)
    {
        cout << "> ";
        string cmdline;  
        getline(cin, cmdline);
        stringstream ss(cmdline);

        string cmd;
        ss >> cmd;

        if (cmd == "view_info")
            cout << "ID: " + device.id << "\nType: " + device.type << endl;
        else if (cmd == "query")
        {
            string scope;
            ss >> scope;
            execute_query(device, scope);
        }
        else if (cmd == "power")
        {
            string mode;
            ss >> mode;
            execute_power(device, mode);
        }
        else if (cmd == "changepassword")
        {
            string current_pass, new_pass;
            ss >> current_pass >> new_pass;
            execute_change_password(device, current_pass, new_pass);
        }
        else if (cmd == "config")
        {
            string param;
            int value;
            ss >> param >> value;
            execute_config(device, param, value);
        }
        else if (cmd == "fertilize")
        {
            int c, v;
            ss >> c >> v;
            execute_fertilize(device, c, v);
        }
        else if (cmd == "unselect")
            break;
        else if (cmd == "help")
            fertilizer_help();
        else
            cerr << "Unknown action. Type \"help\" to see all actions." << endl;
    }
}

void outer_help()
{
    cout << "1. scan : Scanning for devices" << endl;
    cout << "2. view_available : Viewing all available devices for connection" << endl;
    cout << "3. connect <device_id> <password> : Connect to a device in the available device list" << endl;
    cout << "4. view_connected : View all connected device" << endl;
    cout << "5. select <device_id> : Select a connected device for controlling" << endl;
}

int main(int argc, char* argv[])
{
    string id = randstr(ID_SIZE);
    cout << "Application running. ID: " + id << endl;
    cout << "Type \"help\" to see all actions." << endl;
    
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
        else if (cmd == "view_available")
        {
            if (available.empty()) cout << "No device available" << endl;
            else
                for (auto& device : available)
                    cout << device.type + " " + device.id << endl;
        }
        else if (cmd == "view_connected")
        {
            if (connected.empty()) cout << "No device connected" << endl;
            else
                for (auto& device : connected)
                    cout << device.type + " " + device.id << endl;
        }
        else if (cmd == "select")
        {
            string device_id;
            ss >> device_id;
            ssize_t index = find_by_id(connected, device_id);
            if (index < 0) cout << "You have not connect to this device yet" << endl;
            else
            {
                string type = connected[index].type;
                if (type == "sensor") sensor_interface(connected[index]);
                else if (type == "sprinkler") sprinkler_interface(connected[index]);
                else if (type == "light") light_interface(connected[index]);
                else fertilizer_interface(connected[index]);
            }
        }
        else if (cmd == "help")
            outer_help();
        else
            cerr << "Unknown action. Type \"help\" to see all actions." << endl;
    }
    return 0;
}