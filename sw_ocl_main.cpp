
#define VERSION 1

#include <cstdlib>
#include <iostream>
#include <fstream>
#include "sw_ocl.h"
#include <sys/time.h>

using namespace std;

void print_help();
int read_ref_seq(string in_file_name, string &ref_seq, string* query_seqs);

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        print_help();
        return 1;
    }

    // GET INPUT
    string in_file_name = argv[1];
    string out_file_name = argv[2];
    cout << "Opening input file " << in_file_name << "..." << endl;
    string *query_seqs = new string[256];
    string ref_seq;
    int num_queries = read_ref_seq(in_file_name, ref_seq, query_seqs);

    // PRINT INPUT
    cout << "Reference sequence:" << endl;
    cout << ref_seq << endl;
    cout << "Query sequences (" << num_queries << "):" << endl;
    for (int i = 0; i < num_queries; i++)
    {
        cout << query_seqs[i] << endl;
    }
    cout << endl;

    float runtime = 0;
    timeval t1, t2;
    // PROCESS INPUT
    OCLAligner algn = OCLAligner();
    algn.set_ref(ref_seq);
    algn.set_queries(query_seqs, num_queries);

    gettimeofday(&t1, NULL);
    string* aligned = algn.get_aligned();
    gettimeofday(&t2, NULL);
    runtime += (t2.tv_sec -t1.tv_sec) * 1000.0;
    runtime += (t2.tv_usec - t1.tv_usec) / 1000.0;


    // PRINT RESULTS
    /*
    cout << "Alignment:" << endl;
    for (int i = 0; i < num_queries; i++)
    {
        cout << ref_seq << endl;
        cout << aligned[i] << endl;
        cout << endl;
    }
    cout << endl;
    */
    cout << "Runtime: " << runtime << endl;

    return 0;
}

void print_help()
{
    cout << "swocl <inFile.fasta> <outFile.fasta>" << endl;
    cout << "\t- inFile.fasta is the unaligned input file" << endl;
    cout    << "\t\t- This file must contain one reference sequence"
            << endl
            << "\t\t  followed by one or more query sequences in the"
            << endl
            << "\t\t  FASTA format"
            << endl;
    cout    << "\t- outFile.fasta is a filename for the aligned output file"
            << endl;
}

int read_ref_seq(string in_file_name, string &ref_seq, string *query_seqs)
{

    ifstream in_file(in_file_name.c_str());
    string this_line;
    // For ref:
    int num_seqs = 0;
    int num_lines = 0;
    while (getline(in_file, this_line))
    {
        if (num_lines == 1)
        {
            ref_seq = this_line;
        }
        if (num_lines%2 == 1 && num_lines > 1) {
            //cout << "Line " << num_seqs << ": " << this_line << endl;
            query_seqs[num_seqs] = this_line;
            num_seqs += 1;
        }
        num_lines += 1;
    }
    //cout << num_seqs << endl;
    return num_seqs;
}
