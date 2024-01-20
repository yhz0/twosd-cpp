CXX = g++  # or clang++
CXXFLAGS = -std=c++17 -Wall -g -DUNIT_TEST -fopenmp

# Solver related
GUROBI_HOME = C:/soft/gurobi1100/win64
INCPATH = -Iinclude -I$(GUROBI_HOME)/include
LIBPATH = -L$(GUROBI_HOME)/lib
LIBS = -lgurobi110

# Source files
SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)
EXEC = twosd

# Test files
TEST_SRCS = $(filter-out src/main.cpp, $(wildcard src/*.cpp)) $(wildcard tests/*.cpp) external/catch_amalgamated.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_EXEC = run_tests

.PHONY: all clean

# Main Executable
twosd: $(OBJS)
	$(CXX) $(CXXFLAGS)  $(LIBPATH) -o $(EXEC) $(OBJS) $(LIBS)

test: $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(LIBPATH) -o $(TEST_EXEC) $^ $(LIBS)
	./$(TEST_EXEC)

clean:
	rm -f src/*.o tests/*.o $(EXEC) $(TEST_EXEC)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCPATH) -c $< -o $@
