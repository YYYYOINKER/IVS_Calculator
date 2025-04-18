CXX = g++
CXXFLAGS = -std=c++23 -Wall -Iinclude -Isrc -I/usr/include/freetype2
LDFLAGS = -lGL -ldl -lglfw -lX11 -pthread -lfreetype

CLANGXX = clang++
IKOS_ANALYZE = ikos analyze

SOURCES = main.cpp src/TextRenderer.cpp src/glad.c include/tiny_obj_loader.cc
LLVM_SOURCES = main.cpp
TARGET = cube_demo

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(SOURCES) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

analyze:
	$(CLANGXX) -std=c++23 -Wall -Iinclude -Isrc -emit-llvm -c $(LLVM_SOURCES) -o main.bc
	$(IKOS_ANALYZE) main.bc

clean:
	rm -f $(TARGET) main.bc

.PHONY: all clean run analyze

