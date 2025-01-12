
CXX = g++
CXXFLAGS = -g -pthread 

TARGET = 
all: mutex_blkring lockfree_blkring atomic_blkring align_blkring buck_blkring


arm: p64_blkring p64_buckring

# Rule to build the executable
mutex_blkring:
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/testbench.cc -DMUTEX_BLKRING

lockfree_blkring:
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/testbench.cc -DLOCKFREE_BLKRING

atomic_blkring:
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/testbench.cc -DATOMIC_BLKRING

align_blkring:
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/testbench.cc -DALIGN_BLKRING

buck_blkring:
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/testbench.cc -DBUCK_BLKRING

p64_blkring:
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/p64_testbench.o progress64/libprogress64.a -DP64_BLKRING

p64_buckring:
	$(CXX) $(CXXFLAGS) -o bin/$@ testbench/p64_testbench.o progress64/libprogress64.a -DP64_BUCKRING

# Rule to compile source files into object files
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(TARGET)

# Run rule (optional)
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
