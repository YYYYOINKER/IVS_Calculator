CXX := g++
CXXFLAGS := -Wall -Wextra -Iinclude -std=c++14 -pthread
TEST_TARGET := calculator_test

SRC := src/mathlibrary.cpp
TEST_SRC := tests/test.cpp

$(TEST_TARGET): $(SRC:.cpp=.o) $(TEST_SRC:.cpp=.o)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lgtest -lgtest_main

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TEST_TARGET) src/*.o tests/*.o

.PHONY: clean test
