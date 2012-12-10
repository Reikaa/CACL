
#define VERSION 1

#include <cstdlib>
#include <iostream>
#include <fstream>
#include "sw_aligner.h"

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

    // PROCESS INPUT
    SWAligner algn = SWAligner();
    algn.set_ref(ref_seq);
    algn.set_queries(query_seqs, num_queries);
    string* aligned = algn.get_aligned();

    // PRINT RESULTS
    cout << "Alignment:" << endl;
    for (int i = 0; i < num_queries; i++)
    {
        cout << aligned[i] << endl;
    }
    cout << endl;

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
    ref_seq = "ATGC";
    //query_seqs = new string[256];
    query_seqs[0] = "TGCA";
    int num_seqs = 1;
    return num_seqs;
}
