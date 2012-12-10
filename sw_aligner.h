#ifndef SW_ALIGNER_H
#define SW_ALIGNER_H

#include "sw.h"
#include <string>

class SWAligner: public Aligner
{
private:
    int* swarray;
public:
    SWAligner();
    ~SWAligner();
    void set_ref(std::string reference);
    void set_queries(std::string* queries, int num_queries);
    void farm_queries();
    std::string align_query(std::string query);
    std::string* get_aligned();
};

#endif
