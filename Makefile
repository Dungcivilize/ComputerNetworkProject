CC := g++
CXXFLAGS := -std=c++17

APP_SRC := application/main.cpp
APP_BIN := application/app

SENSOR_SRC := sensor/main.cpp
SENSOR_BIN := sensor/sensor

.PHONY: all app sensor clean

all: app sensor

app: $(APP_BIN)

sensor: $(SENSOR_BIN)

$(APP_BIN): $(APP_SRC)
	$(CC) $(CXXFLAGS) -o $@ $<

$(SENSOR_BIN): $(SENSOR_SRC)
	$(CC) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(APP_BIN) $(SENSOR_BIN)
