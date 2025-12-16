#ifndef HANDLECONTROL_HPP
#define HANDLECONTROL_HPP

#include <vector>
#include "../common/index.hpp"

void power_control(std::vector<Device*>& devices);
void take_control(std::vector<Device*>& devices);
void set_timer(std::vector<Device*>& devices);
void cancel_control(std::vector<Device*>& devices);

#endif // HANDLECONTROL_HPP