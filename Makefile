CXX = g++  # or clang++
CXXFLAGS = -std=c++14 -Wall -Iinclude

# Source files
SRCS = src/smps.cpp src/sparse_matrix.cpp src/main.cpp
OBJS = $(SRCS:.cpp=.o)

# Test files
TEST_SRCS = tests/sparse_matrix_test.cpp src/sparse_matrix.cpp external/catch_amalgamated.cpp
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
	rm -f src/*.o tests/*.o main $(TEST_EXEC)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
