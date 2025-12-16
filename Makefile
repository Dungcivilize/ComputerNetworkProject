CXX := g++
CXXFLAGS := -std=c++11 -Wall -pthread
INCLUDES := -I. -Icommon -Idependencies -Ihandler

# Gather sources from the current project layout
COMMON_SRCS := $(wildcard common/*.cpp)
DEPS_SRCS := $(wildcard dependencies/*.cpp)
HANDLER_SRCS := $(wildcard handler/*.cpp)

APP_SRCS := app.cpp $(COMMON_SRCS) $(DEPS_SRCS) $(HANDLER_SRCS)
SENSOR_SRCS := sensor.cpp dependencies/streamtransmission.cpp dependencies/utils.cpp

APP_OBJS := $(APP_SRCS:.cpp=.o)
SENSOR_OBJS := $(SENSOR_SRCS:.cpp=.o)

all: app sensor

.PHONY: all clean

app: $(APP_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(APP_OBJS)

sensor: $(SENSOR_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(SENSOR_OBJS)

# Generic rule for compiling .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -f *.o common/*.o dependencies/*.o handler/*.o app sensor
