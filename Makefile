CXX := g++
CXXFLAGS := -std=c++17 -pthread

APP_SRC := application/main.cpp
APP_BIN := application/app

SENSOR_SRC := sensor/main.cpp
SENSOR_BIN := sensor/sensor

SPRINKLER_SRC := sprinkler/main.cpp
SPRINKLER_BIN := sprinkler/sprinkler

FERTILIZER_SRC := fertilizer/main.cpp
FERTILIZER_BIN := fertilizer/fertilizer

LIGHT_SRC := light/main.cpp
LIGHT_BIN := light/light

BINS := $(APP_BIN) $(SENSOR_BIN) $(SPRINKLER_BIN) $(FERTILIZER_BIN) $(LIGHT_BIN)

.PHONY: all app sensor sprinkler fertilizer light clean

all: $(BINS)

app: $(APP_BIN)
sensor: $(SENSOR_BIN)
sprinkler: $(SPRINKLER_BIN)
fertilizer: $(FERTILIZER_BIN)
light: $(LIGHT_BIN)

$(APP_BIN): $(APP_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(SENSOR_BIN): $(SENSOR_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(SPRINKLER_BIN): $(SPRINKLER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(FERTILIZER_BIN): $(FERTILIZER_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(LIGHT_BIN): $(LIGHT_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(BINS)
