CXX := g++
CXXFLAGS := -std=c++17 -Wall -pthread
INCLUDES := -I. -Icommon -Idependencies -Ihandler -Isensorserver

# Gather sources from the current project layout
COMMON_SRCS := $(wildcard common/*.cpp)
DEPS_SRCS := $(wildcard dependencies/*.cpp)
HANDLER_SRCS := $(wildcard handler/*.cpp)

APP_SRCS := app.cpp $(COMMON_SRCS) $(DEPS_SRCS) $(HANDLER_SRCS)
SENSOR_SRCS := $(DEPS_SRCS)
TEST_SENSOR_SRCS := tests/test_sensor.cpp
TEST_SENSOR_CLIENT_SRCS := tests/test_sensor_client.cpp
TEST_EMULATOR_SENSOR_SRCS := tests/testEmulatorSensor.cpp
TEST_EMULATOR_SRCS := tests/testEmulator.cpp

APP_OBJS := $(APP_SRCS:.cpp=.o)
SENSOR_OBJS := $(SENSOR_SRCS:.cpp=.o)
TEST_SENSOR_OBJS := $(TEST_SENSOR_SRCS:.cpp=.o)
TEST_SENSOR_CLIENT_OBJS := $(TEST_SENSOR_CLIENT_SRCS:.cpp=.o)
TEST_EMULATOR_SENSOR_OBJS := $(TEST_EMULATOR_SENSOR_SRCS:.cpp=.o)
TEST_EMULATOR_OBJS := $(TEST_EMULATOR_SRCS:.cpp=.o)

all: app test_sensor test_sensor_client testEmulatorSensor testEmulator

.PHONY: all clean

app: $(APP_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(APP_OBJS)

# `sensor` is a header-only class now; build test binaries that use it instead.

test_sensor: $(TEST_SENSOR_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o tests/test_sensor $(TEST_SENSOR_OBJS)

test_sensor_client: $(TEST_SENSOR_CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o tests/test_sensor_client $(TEST_SENSOR_CLIENT_OBJS)

testEmulatorSensor: $(TEST_EMULATOR_SENSOR_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(TEST_EMULATOR_SENSOR_OBJS)

testEmulator: $(TEST_EMULATOR_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(TEST_EMULATOR_OBJS)

# Generic rule for compiling .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f *.o */*.o common/*.o dependencies/*.o handler/*.o sensorserver/*.o tests/*.o
	[ -f app ] && rm -f app
	[ -f sensor ] && rm -f sensor
	[ -f tests/test_sensor ] && rm -f tests/test_sensor
	[ -f tests/test_sensor_client ] && rm -f tests/test_sensor_client
	[ -f testEmulatorSensor ] && rm -f testEmulatorSensor
	[ -f testEmulator ] && rm -f testEmulator
