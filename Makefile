CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread
TARGET = safestate

all: $(TARGET)

$(TARGET): main.cpp resource_manager.h
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp

clean:
	rm -f $(TARGET) *.dot

.PHONY: all clean
