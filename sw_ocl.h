#ifndef OCL_ALIGNER_H
#define OCL_ALIGNER_H

#include "sw.h"
#include <string>
#if defined(__APPLE__) || defined(APPLE)
    #include <OpenCL/OpenCL.h>
#else
    //#include <CL/opencl.h>
    #include <CL/cl.h>
    #include <CL/cl_ext.h>
    //#define FLOAT_ZERO_TWO
#endif

class OCLAligner: public Aligner
{
private:
    cl_context ctx;
    cl_kernel basic_align;
    cl_command_queue queue;
    size_t global_work_size;
    size_t local_work_size;
    cl_int err_num;
    cl_program prog;
    const char* kernel_name;

    cl_mem d_ref;
    cl_mem d_query;
    cl_mem d_backpointers;
    cl_mem d_swarray;

    int setup_context();
    int compile_kernel(const char* kernel_name, cl_kernel &kernel);
    int create_buffer(cl_mem &d_buf, void* buffer, size_t size);
    int launch_kernel(cl_kernel kernel);
    int read_buffer(cl_mem &d_buf, void* data, size_t size);
    int round64(int string_len) { return (string_len/64 + 1) * 64;};
public:
    OCLAligner();
    ~OCLAligner();
    void set_ref(std::string reference);
    void set_queries(std::string* queries, int num_queries);
    void farm_queries();
    std::string align_query(std::string query);
    std::string* get_aligned();
};

#endif
