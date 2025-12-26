#include "utils.hpp"

int main()
{
    const string buf = "SENSOR:sensor_id_0123:Hello World:1:2:3:4";
    vector<string> tokens = parse_info_message(buf);
    for (auto& token : tokens)
        cout << token << endl;
    return 0;
}