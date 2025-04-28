CXX := g++
CXXFLAGS := -Wall -Iinclude -std=c++14
TARGET := calculator
TEST_TARGET := calculator_test

SRC := src/main.cpp src/mathlibrary.cpp
TEST_SRC := tests/test.cpp src/mathlibrary.cpp
OBJ := $(SRC:.cpp=.o)
TEST_OBJ := $(filter-out src/main.o, $(OBJ))

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TEST_TARGET): $(TEST_OBJ) tests/test.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lgtest -lgtest_main -pthread

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET) src/*.o tests/*.o

.PHONY: clean test
