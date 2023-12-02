CXX = g++  # or clang++
CXXFLAGS = -std=c++17 -Wall -g -Iinclude

# Source files
MAIN_SRCS = $(wildcard src/*.cpp)
MAIN_OBJS = $(MAIN_SRCS:.cpp=.o)
MAIN_EXEC = main

# Test files
TEST_SRCS = $(filter-out src/main.cpp, $(wildcard src/*.cpp)) $(wildcard tests/*.cpp) external/catch_amalgamated.cpp
TEST_OBJS = $(TEST_SRCS:.cpp=.o)
TEST_EXEC = run_tests

.PHONY: all clean test

all: main

main: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

test: $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $(TEST_EXEC) $^
	./$(TEST_EXEC)

clean:
	rm -f src/*.o tests/*.o $(MAIN_EXEC) $(TEST_EXEC)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
