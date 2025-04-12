CXX = g++
CXXFLAGS = -std=c++23 -Wall -Iinclude -Isrc -I/usr/include -lGL -ldl -lglfw -lX11 -pthread

SOURCES = main.cpp src/glad.c
TARGET = cube_demo

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(SOURCES) -o $(TARGET) $(CXXFLAGS)

clean:
	rm -f $(TARGET)
