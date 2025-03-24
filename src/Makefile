# Keep existing calculator build rules (for calc.cpp)
calc: calc.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Add OpenGL build rules (new)
CXXFLAGS += -std=c++17
LDFLAGS += -lglfw -lGL

opengl_test: opengl_test.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Combined 'all' target
all: calc opengl_test
