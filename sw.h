#ifndef SW_H
#define SW_H

#include <string>

class Aligner
{
protected:
    std::string reference;
    std::string* queries;
    int num_queries;
    std::string* alignment;
public:
    virtual void set_ref(std::string reference) = 0;
    virtual void set_queries(std::string* queries, int num_queries) = 0;
    virtual std::string* get_aligned() = 0;
};

#endif
