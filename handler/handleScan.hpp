#include <vector>
#include "../common/index.hpp"
#include "../dependencies/index.hpp"
#include "../common/dataStructure.hpp"

// Return allocated Device* objects; caller must manage lifetime.
std::vector<Device*> scan_devices(uint16_t port);