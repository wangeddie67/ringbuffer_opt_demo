
CXX = g++
CXXFLAGS = -g -pthread 
#CXXFLAGS = -std=c++17 -Wall -O2

SRCS = testbench/testbench.cc \
	testbench/p64_testbench.cc \
       srcs/mutex_blkring.cc \
       srcs/mutex_nonblkring.cc \
	   srcs/align_blkring.cc \
	   srcs/atomic_blkring.cc
OBJS = $(SRCS:.cc=.o) \
	   $(SRCS:.cc=_align.o)

TARGET = mutex_blkring \
		 mutex_nonblkring \
		 align_blkring \
		 atomic_blkring

all: $(TARGET)

arm: p64_blkring

# Rule to build the executable
mutex_blkring: $(OBJS)
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/testbench_align.o srcs/mutex_blkring_align.o

mutex_nonblkring: $(OBJS)
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/testbench.o srcs/mutex_nonblkring.o

atomic_blkring: $(OBJS)
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/testbench.o srcs/atomic_blkring.o

align_blkring: $(OBJS)
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/testbench_align.o srcs/align_blkring_align.o

p64_blkring: $(OBJS)
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/p64_testbench.o progress64/libprogress64.a


# Rule to compile source files into object files
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

%_align.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@ -DALIGN_BUFFER

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

# Run rule (optional)
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
