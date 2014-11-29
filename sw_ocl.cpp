
#include <iostream>
#include <string.h>
#include <sys/time.h>
#include "sw_ocl.h"
#include <sys/time.h>

using namespace std;

#define STRINGIFY(src) #src

#define kernel_header "#pragma OPENCL EXTENSION cl_khr_fp64 : enable;\n\n"

inline const char* Kernels()
{
    static const char* kernels =
        kernel_header
        #include "sw_ocl.cl"
        ;
    return kernels;
}

OCLAligner::OCLAligner()
{
    this->setup_context();
    this->kernel_name = "basic_align";
    compile_kernel(kernel_name, this->basic_align);
}

OCLAligner::~OCLAligner()
{

}

void OCLAligner::set_ref(string reference)
{
    this->reference = reference.insert(0, "-");
}

void OCLAligner::set_queries(string* queries, int num_queries)
{
    this->num_queries = num_queries;
    this->queries = queries;
    //alignment = (string*)malloc(sizeof(string*) * num_queries);
    alignment = new string[num_queries];
}

void OCLAligner::farm_queries()
{
    for (int i = 0; i < num_queries; i++)
    {
        alignment[i] = align_query(queries[i].insert(0, "-"));
    }
}

string OCLAligner::align_query(string query)
{
    string alignedQuery = "";
    float runtime = 0;
    timeval t1, t2;

    int refsize = reference.size();
    int roundrefsize = round64(reference.size());
    //cout << "Round reference size: " << roundrefsize << endl;
    int querysize = query.size();
    int roundquerysize = round64(query.size());
    //cout << "Round query size: " << roundquerysize << endl;
    //this->global_work_size = roundquerysize * roundrefsize;
    this->global_work_size = roundquerysize;
    this->local_work_size = roundquerysize;
    //char* backpointers = new char[query.size() * reference.size()];
    char* backpointers = new char[roundquerysize * roundrefsize];
    for (int row = 0; row < roundquerysize; row++)
    {
        for (int col = 0; col < roundrefsize; col++)
        {
          backpointers[row*roundrefsize + col] = 0;
        }
    }
    //int* swarray = new int[query.size() * reference.size()];
    int* swarray = new int[roundquerysize * roundrefsize];
    for (int row = 0; row < roundquerysize; row++)
    {
        for (int col = 0; col < roundrefsize; col++)
        {
            swarray[row*roundrefsize + col] = 0;
        }
    }
    create_buffer(d_ref, (char*)reference.c_str(), roundrefsize);
    create_buffer(d_query, (char*)query.c_str(), roundquerysize);
    create_buffer(d_backpointers, backpointers, roundquerysize * roundrefsize);
    create_buffer(d_swarray, swarray, roundquerysize * roundrefsize * sizeof(int));
    err_num  = clSetKernelArg(basic_align, 0, sizeof(cl_mem), (void *) &d_ref);
    err_num  |= clSetKernelArg(basic_align, 1, sizeof(cl_mem), (void *) &d_query);
    err_num  |= clSetKernelArg( basic_align,
                                2, sizeof(cl_mem), (void *) &d_backpointers);
    err_num  |= clSetKernelArg(basic_align, 3, sizeof(cl_mem), (void *) &d_swarray);
    err_num  |= clSetKernelArg(basic_align, 4, roundquerysize * sizeof(cl_int) * 3, NULL);
    err_num  |= clSetKernelArg(basic_align, 5, sizeof(cl_int), (void *) &roundrefsize);
    err_num  |= clSetKernelArg(basic_align, 6, sizeof(cl_int), (void *) &roundquerysize);
    if (err_num != CL_SUCCESS)
    {
        cout << "kernel arg set fail" << endl;
        exit(err_num);
    }
    gettimeofday(&t1, NULL);
    launch_kernel(basic_align);
    gettimeofday(&t2, NULL);
    read_buffer(d_backpointers, backpointers, roundrefsize * roundquerysize);
    read_buffer(d_swarray, swarray, roundrefsize * roundquerysize * sizeof(int));

    runtime += (t2.tv_sec -t1.tv_sec) * 1000.0;
    runtime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    cout << "Core time: " << runtime << endl;
    /*
    for (int row = 0; row < querysize; row++)
    {
        for (int col = 0; col < refsize; col++)
        {
          cout << swarray[row*roundrefsize + col] << ", ";
        }
        cout << endl;
    }
    */
    // FIND THE END
    int bestScore = 0;
    int bestRow = 0;
    int bestCol = 0;
    //for (int row = 0; row < query.size(); row++)
    for (int row = 0; row < querysize; row++)
    {
        //for (int col = 0; col < reference.size(); col++)
        for (int col = 0; col < refsize; col++)
        {
            if (swarray[row*roundrefsize + col] > bestScore)
            {
                bestScore = swarray[row*roundrefsize + col];
                bestRow = row;
                bestCol = col;
            }
            //cout << swarray[row*reference.size() + col] << ", ";
        }
        //cout << endl;
    }
    // BACKTRACK
    int endCol = bestCol;
    while (bestScore > 0)
    {
        // END OF THE ROAD
        //if (backpointers[bestRow*reference.size() + bestCol] == 0)
        if (backpointers[bestRow*roundrefsize + bestCol] == 0)
        {
            bestScore = 0;
            alignedQuery += query[bestRow];
        }
        // MATCH/MISMATCH
        //else if (backpointers[bestRow*reference.size() + bestCol] == 1)
        else if (backpointers[bestRow*roundrefsize + bestCol] == 1)
        {
            alignedQuery += query[bestRow];
            bestRow -= 1;
            bestCol -= 1;
            //bestScore = swarray[bestRow * reference.size() + bestCol];
            bestScore = swarray[bestRow * roundrefsize + bestCol];
        }
        // DELETION (move down)
        //else if (backpointers[bestRow*reference.size() + bestCol] == 2)
        else if (backpointers[bestRow*roundrefsize + bestCol] == 2)
        {
            // XXX this will remove something from the query
            bestRow -= 1;
            //bestScore = swarray[bestRow * reference.size() + bestCol];
            bestScore = swarray[bestRow * roundrefsize + bestCol];
        }
        // INSERTION (move right)
        //else if (backpointers[bestRow*reference.size() + bestCol] == 3)
        else if (backpointers[bestRow*roundrefsize + bestCol] == 3)
        {
            alignedQuery += "-";
            bestCol -= 1;
            //bestScore = swarray[bestRow * reference.size() + bestCol];
            bestScore = swarray[bestRow * roundrefsize + bestCol];
        }
    }
    alignedQuery = alignedQuery.substr(0, alignedQuery.size()-1);
    while (bestCol > 0)
    {
        alignedQuery += "-";
        bestCol -= 1;
    }
    for (int i = 0; i < alignedQuery.size()/2; i++)
    {
        char temp = alignedQuery[i];
        alignedQuery[i] = alignedQuery[alignedQuery.size() - 1 - i];
        alignedQuery[alignedQuery.size() - 1 - i] = temp;
    }
    while (endCol < (reference.size() - 1))
    {
        alignedQuery += "-";
        endCol += 1;
    }
    if (alignedQuery.size() < reference.size())
    {
        alignedQuery.insert(0, "-");
    }
    return alignedQuery;
}

string* OCLAligner::get_aligned()
{
    farm_queries();
    return alignment;
}

int OCLAligner::create_buffer(cl_mem &d_buf, void* data, size_t size)
{
    d_buf = clCreateBuffer( ctx,
                    CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                    size,
                    data,
                    &err_num);
    if (err_num != CL_SUCCESS)
    {
        cout << "make buffer fail" << endl;
        exit(err_num);
    }
    clFinish(queue);
    /*
    err_num = clEnqueueWriteBuffer( queue,
                                    d_buf,
                                    CL_TRUE,
                                    0,
                                    size,
                                    data,
                                    0,
                                    NULL,
                                    NULL
            );
    if (err_num != CL_SUCCESS)
    {
        cout << "make buffer fail" << endl;
        exit(err_num);
    }
            */
    return 0;
}

int OCLAligner::read_buffer(cl_mem &d_buf, void* data, size_t size)
{
    err_num = clEnqueueReadBuffer(  queue,
                                    d_buf,
                                    CL_TRUE,
                                    0,
                                    size,
                                    data,
                                    0,
                                    NULL,
                                    NULL
            );
    if (err_num != CL_SUCCESS)
    {
        cout << "read fail" << endl;
        exit(err_num);
    }
    return 0;
}

int OCLAligner::launch_kernel(cl_kernel kernel)
{
    err_num = clEnqueueNDRangeKernel(   queue,
                                        kernel,
                                        1,
                                        0,
                                        &global_work_size,
                                        &local_work_size,
                                        0,
                                        NULL,
                                        NULL);
    if (err_num != CL_SUCCESS)
    {
        cout << "kernel launch fail" << endl;
        exit(err_num);
    }
    clFinish(queue);
    return 0;
}


int OCLAligner::setup_context()
{
    cl_platform_id plat = NULL;
    cl_device_id *devices = NULL;
    cl_device_id device = NULL;
    cl_uint dev_count = 0;
    err_num = CL_SUCCESS;

    err_num = clGetPlatformIDs(1, &plat, NULL);
    if (err_num != CL_SUCCESS)
    {
        cout << "Plat fail" << endl;
        exit(err_num);
    }

    // Dev setup
    err_num = clGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, 0, NULL, &dev_count);
    devices = (cl_device_id *)malloc(dev_count * sizeof(cl_device_id));
    err_num = clGetDeviceIDs(plat, CL_DEVICE_TYPE_GPU, dev_count, devices, NULL);

    device = devices[1]; // XXX set back down to 0
    if (err_num != CL_SUCCESS)
    {
        cout << "Dev fail" << endl;
        exit(err_num);
    }

    // Context setup
    // 1 == my device count (arbitrary)
    this->ctx = clCreateContext(0, 1, &device, NULL, NULL, &err_num);
    if (err_num != CL_SUCCESS)
    {
        cout << "Ctx fail" << endl;
        exit(err_num);
    }

    // get device info
    size_t returned_size = 0;
    cl_char vendor_name[1024] = {0};
    cl_char device_name[1024] = {0};
    size_t wg_max;
    size_t lm_max;
    err_num = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(vendor_name),
                             vendor_name, &returned_size);
    err_num |= clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name),
                              device_name, &returned_size);
    err_num |= clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(wg_max),
                              &wg_max, &returned_size);
    err_num |= clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(wg_max),
                              &lm_max, &returned_size);
    if (err_num != CL_SUCCESS)
    {
        cout << "Name fetch fail" << endl;
        exit(err_num);
    }
    //printf("Connecting to %s %s...\n", vendor_name, device_name);
    cout << "Connecting to " << vendor_name << " " << device_name << "..." << endl;
    //printf("Max work group size: %"PRIuPTR"\n", wg_max);
    cout << "Max work group size: " << wg_max << endl;
    cout << "Local memory size: " << lm_max << endl;
    //local_work_size = wg_max;
    //local_work_size = 256;

    // queue setup
    this->queue = clCreateCommandQueue(   ctx,
                                    device,
                                    0,
                                    &err_num);
    if (err_num != CL_SUCCESS)
    {
        cout << "queue fail" << endl;
        exit(err_num);
    }

    // prog setup
    const char* source = Kernels();
    this->prog = clCreateProgramWithSource(ctx, 1, &source, NULL, &err_num);
    if (err_num != CL_SUCCESS)
    {
        cout << "compile fail" << endl;
        exit(err_num);
    }

    // build program
    err_num = clBuildProgram(prog, 1, &device, "-cl-fast-relaxed-math -cl-mad-enable", NULL, NULL);
    if (err_num != CL_SUCCESS)
    {
        cout << "build fail " << err_num << endl;
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char *log = (char *) malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        //printf("%s\n", log);
        cout << log << endl;
        exit(err_num);
    }
    return 0;
}

int OCLAligner::compile_kernel(const char* kernel_name, cl_kernel &kernel)
{
    // kernel setup
    kernel = clCreateKernel(prog, kernel_name, &err_num);
    if (err_num != CL_SUCCESS)
    {
        cout << "make kernel fail" << endl;
        cout << err_num << endl;
        exit(err_num);
    }
    return 0;
}
