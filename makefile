
CXX = g++
CXXFLAGS = -g -pthread 
#CXXFLAGS = -std=c++17 -Wall -O2

SRCS = testbench/testbench.cc \
       srcs/mutex_blkring.cc \
       srcs/mutex_nonblkring.cc \
	   srcs/align_blkring.cc \
	   srcs/atomic_blkring.cc
OBJS = $(SRCS:.cc=.o)

TARGET = mutex_blkring \
		 mutex_nonblkring \
		 align_blkring \
		 atomic_blkring

all: $(TARGET)

# Rule to build the executable
mutex_blkring: $(OBJS)
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/testbench.o srcs/mutex_blkring.o

mutex_nonblkring: $(OBJS)
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/testbench.o srcs/mutex_nonblkring.o

align_blkring: $(OBJS)
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/testbench.o srcs/align_blkring.o

atomic_blkring: $(OBJS)
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/testbench.o srcs/atomic_blkring.o

# Rule to compile source files into object files
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

# Run rule (optional)
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
