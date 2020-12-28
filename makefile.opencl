
CFLAGS   += -isystem $(KALRAY_TOOLCHAIN_DIR)/include
CXXFLAGS += -isystem $(KALRAY_TOOLCHAIN_DIR)/include
LDFLAGS  += -L$(KALRAY_TOOLCHAIN_DIR)/lib -lOpenCL

# with some version of g++,
# two steps generation (compile and link) is needed to avoid link error !

# link
opencl-sample1: opencl-sample1.o
	$(CXX) $< -o $@ $(LDFLAGS)

# compile
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# clean
clean:
	rm -f *.o opencl-sample1
