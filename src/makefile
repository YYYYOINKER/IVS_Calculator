CXX := g++
CXXFLAGS := -Wall -Wextra -Iinclude -std=c++23 -pthread
PROFILE_FLAGS := -g -O0 -pg
TEST_TARGET := calculator_test
STDDEV_TARGET := profiling

SRC := src/mathlibrary.cpp
TEST_SRC := tests/test.cpp
STDDEV_SRC := profiling.cpp

$(TEST_TARGET): $(SRC:.cpp=.o) $(TEST_SRC:.cpp=.o)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lgtest -lgtest_main

$(STDDEV_TARGET): $(STDDEV_SRC) $(SRC)
	$(CXX) $(PROFILE_FLAGS) -o $@ $^ $(CXXFLAGS)

input10.txt:
	seq 1 10 > input10.txt

input1000.txt:
	seq 1 1000 > input1000.txt

input1000000.txt:
	seq 1 1000000 > input1000000.txt

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

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TEST_TARGET) $(STDDEV_TARGET) src/*.o tests/*.o \
	gmon*.out report*.txt input*.txt

.PHONY: clean test stddev
