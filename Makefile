# Compiler
CXX := g++

# Compiler flags
CXXFLAGS := -Wall -Wextra -pedantic -std=c++11


# Source files directory
SRC_DIR := algo

# Source files1
SRCS := $(wildcard $(SRC_DIR)/*.cpp)

# Object files
OBJS := $(SRCS:.cpp=.o)

# Target executable
TARGET := main

# Rule to build the target executable
$(TARGET): $(OBJS) main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

# Rule to build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)