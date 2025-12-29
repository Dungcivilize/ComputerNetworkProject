CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pthread
INCLUDES := -I. -Icommon -Idependencies -Ihandler -Isensorserver -Itests

# Source groups
COMMON_SRCS := $(wildcard common/*.cpp)
DEPS_SRCS := $(wildcard dependencies/*.cpp)
HANDLER_SRCS := $(wildcard handler/*.cpp)
ROOT_SRCS := app.cpp sensor.cpp
SENSORSERVER_SRCS := $(wildcard sensorserver/*.cpp)

# Test sources
TEST_SRCS := tests/test_sensor.cpp tests/test_sensor_client.cpp tests/testEmulatorSensor.cpp tests/testEmulator.cpp

# Targets
APP_SRCS := app.cpp $(COMMON_SRCS) $(DEPS_SRCS) $(HANDLER_SRCS) $(SENSORSERVER_SRCS)
SENSOR_SRCS := sensor.cpp $(COMMON_SRCS) $(DEPS_SRCS) $(SENSORSERVER_SRCS)

APP_OBJS := $(APP_SRCS:.cpp=.o)
SENSOR_OBJS := $(SENSOR_SRCS:.cpp=.o)

TEST_BINARIES := tests/test_sensor tests/test_sensor_client tests/testEmulatorSensor tests/testEmulator

all: app sensor $(TEST_BINARIES)

.PHONY: all clean

app: $(APP_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(APP_OBJS)

sensor: $(SENSOR_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(SENSOR_OBJS)

tests/test_sensor: tests/test_sensor.cpp $(DEPS_SRCS) $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

tests/test_sensor_client: tests/test_sensor_client.cpp $(DEPS_SRCS) $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

tests/testEmulatorSensor: tests/testEmulatorSensor.cpp $(DEPS_SRCS) $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

tests/testEmulator: tests/testEmulator.cpp $(DEPS_SRCS) $(COMMON_SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Pattern rule for compiling sources
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f $(APP_OBJS) $(SENSOR_OBJS) */*.o common/*.o dependencies/*.o handler/*.o sensorserver/*.o tests/*.o
	rm -f app sensor $(TEST_BINARIES)
