CXX := g++
CXXFLAGS := -std=c++17 -Wall -pthread
INCLUDES := -I. -Icommon -Idependencies -Ihandler -Isensor

# Gather sources from the current project layout
COMMON_SRCS := $(wildcard common/*.cpp)
DEPS_SRCS := $(wildcard dependencies/*.cpp)
HANDLER_SRCS := $(wildcard handler/*.cpp)

APP_SRCS := app.cpp $(COMMON_SRCS) $(DEPS_SRCS) $(HANDLER_SRCS)
SENSOR_SRCS := sensor/sensor.cpp $(DEPS_SRCS)
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

all: app sensor test_sensor test_sensor_client testEmulatorSensor testEmulator

.PHONY: all clean

app: $(APP_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(APP_OBJS)

sensor: $(SENSOR_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(SENSOR_OBJS)

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
	rm -f *.o */*.o common/*.o dependencies/*.o handler/*.o sensor/*.o tests/*.o app sensor tests/test_sensor tests/test_sensor_client tests/testEmulatorSensor tests/testEmulator
