UNAME := $(shell uname)
CXX = clang++
ifeq ($(UNAME), Linux)
CXX = g++
endif

flags = -O3
# omp and clang don't get along
#all_flags = -O3 -Wall -Werror -Wno-unknown-pragmas
all_flags = -O3 -Wall -Wno-unknown-pragmas

.PHONY: all clean
all: cpu ocl

#core_objs = sw.o
cpu_objs = sw_cpu_main.o sw_cpu.o
gpu_objs = sw_ocl_main.o sw_ocl.o


###### CPU Version ######

# is this neccessary, or will the catch all compile it?
#sw_aligner.o: sw_aligner.cpp
#    $(CXX) $(flags) -c -o sw_aligner.o sw_aligner.cpp

#cpu: $(core_objs) $(cpu_objs)
cpu: $(cpu_objs)
	$(CXX) $(flags) -o swcpu $^

###### GPU Version ######

ocl: flags += -DOCL
ifeq ($(UNAME), Linux)
    ocl: ocl_lib = -lOpenCL
else
    ocl: ocl_lib = -framework OpenCL
endif

# is this neccessary, or will the catch all compile it?
#swocl.o: swocl.cpp
	#$(CXX) $(flags) -c -o $@ $<

#ocl: $(gpu_objs) $(core_objs)
ocl: $(gpu_objs)
	$(CXX) $(flags) -o swocl $^ $(ocl_lib)

###### ETC ######

%.o: %.cpp
	$(CXX) $(all_flags) -c -o $@ $<

clean:
	-rm -f *.o swocl swcpu
