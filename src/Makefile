CXX = g++
CLANGXX = clang++

CXXFLAGS = -std=c++23 -Wall -Iinclude -Isrc -I/usr/include/freetype2 -pthread
LDFLAGS = -lGL -ldl -lglfw -lX11 -pthread -lfreetype

PROFILE_FLAGS = -g -O0 -pg

TARGET = calculatorGUI
SOURCES = main_gui.cpp src/TextRenderer.cpp src/glad.c include/tiny_obj_loader.cc src/mathlibrary.cpp

TEST_TARGET = calculator_test
TEST_SRC = tests/test.cpp
MATHLIB_SRC = src/mathlibrary.cpp

STDDEV_TARGET = profiling
STDDEV_SRC = profiling.cpp

LLVM_SOURCES = main_gui.cpp

DOXYFILE = Doxyfile
PACK_NAME = xdurkal00_xvihond00_xmihalp01.zip

OBJS = $(MATHLIB_SRC:.cpp=.o) $(TEST_SRC:.cpp=.o)

.PHONY: all clean run test stddev doc pack

.DEFAULT_GOAL := all

# Default target
all: $(TARGET) $(TEST_TARGET) $(STDDEV_TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

# Test target
$(TEST_TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lgtest -lgtest_main

test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Profiling build and run
$(STDDEV_TARGET): $(STDDEV_SRC) $(MATHLIB_SRC)
	$(CXX) $(PROFILE_FLAGS) -o $@ $^ $(CXXFLAGS)

input10.txt:
	seq 1 10 > $@

input1000.txt:
	seq 1 1000 > $@

input1000000.txt:
	seq 1 1000000 > $@

stddev: $(STDDEV_TARGET) input10.txt input1000.txt input1000000.txt
	./$(STDDEV_TARGET) < input10.txt
	mv gmon.out gmon10.out
	gprof $(STDDEV_TARGET) gmon10.out > report10.txt

	./$(STDDEV_TARGET) < input1000.txt
	mv gmon.out gmon1000.out
	gprof $(STDDEV_TARGET) gmon1000.out > report1000.txt

	./$(STDDEV_TARGET) < input1000000.txt
	mv gmon.out gmon1000000.out
	gprof $(STDDEV_TARGET) gmon1000000.out > report1000000.txt

# Documentation
doc:
	doxygen $(DOXYFILE)

# Packaging
pack: clean doc
	cd ..; \
	mkdir xdurkal00_xmihalp01_xvihond00; \
	cd xdurkal00_xmihalp01_xvihond00; \
	mkdir repo; \
	mkdir install; \
	mkdir build; \
	mkdir doc; \
	mv ../src/docs/* doc/; \
	cd build; \
	cmake -DINSTALL_TYPE=app ../.. && cpack; \
	cmake -DINSTALL_TYPE=stddev ../.. && cpack; \
	mv *.deb ../install/; \
	cd ../repo; \
	git clone https://github.com/YYYYOINKER/IVS_Calculator; \
	cd ../..; \
	zip -r xdurkal00_xmihalp01_xvihond00.zip xdurkal00_xmihalp01_xvihond00; \
	rm -rf xdurkal00_xmihalp01_xvihond00

help:
	@echo "Available targets:"
	@echo "  all      – Build calculator, tests, and profiling tool"
	@echo "  run      – Run the GUI calculator"
	@echo "  test     – Run unit tests"
	@echo "  stddev   – Profile with various inputs using gprof"
	@echo "  doc      – Generate documentation with Doxygen"
	@echo "  clean    – Remove build files, reports, and temp files"
	@echo "  pack     – Package the entire repo (including .git) for submission"

# Cleanup
clean:
	rm -f $(TARGET) $(TEST_TARGET) $(STDDEV_TARGET)
	rm -f src/*.o tests/*.o *.bc
	rm -f gmon*.out report*.txt input*.txt
	rm -rf docs/
	rm -f *.zip

