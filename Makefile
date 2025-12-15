CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread

# Source files
APP_SRC = app.cpp module/common.cpp module/register.cpp module/index.hpp
SENSOR_SRC = sensor.cpp

# Output binaries
APP_BIN = app
SENSOR_BIN = sensor

all: $(APP_BIN) $(SENSOR_BIN)

$(APP_BIN): $(APP_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(APP_SRC)

$(SENSOR_BIN): $(SENSOR_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $(SENSOR_SRC)

clean:
	rm -f $(APP_BIN) $(SENSOR_BIN)
