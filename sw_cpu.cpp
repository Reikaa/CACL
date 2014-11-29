
#include "sw_cpu.h"
#include <iostream>
#include <sys/time.h>
using namespace std;

SWAligner::SWAligner()
{

}

SWAligner::~SWAligner()
{

}

void SWAligner::set_ref(string reference)
{
    this->reference = reference;
}

void SWAligner::set_queries(string* queries, int num_queries)
{
    this->num_queries = num_queries;
    this->queries = queries;
    //alignment = (string*)malloc(sizeof(string*) * num_queries);
    alignment = new string[num_queries];
}

void SWAligner::farm_queries()
{
    for (int i = 0; i < num_queries; i++)
    {
        alignment[i] = align_query(queries[i]);
    }
}

string SWAligner::align_query(string query)
{
    // XXX Does this need to be a private variable. Can it just be made and
    // destroyed here?
    string alignedQuery = "";
    float runtime = 0;
    timeval t1, t2;
    swarray = new int[query.size() * reference.size()];
    char* backpointers = new char[query.size() * reference.size()];
    gettimeofday(&t1, NULL);
    // ALIGN
    for (unsigned int row = 0; row < query.size(); row++)
    {
        for (unsigned int col = 0; col < reference.size(); col++)
        {
            //int start = 0;
            //int bestScore = start;
            int bestScore = 0;
            char bestSource = 0;

            // Possible moves:
            int match = 0;
            int insertion = 0;
            int deletion = 0;

            // Match/Mismatch
            if (row > 0 && col > 0)
            {
                if (query[row] == reference[col])
                    match = swarray[(row-1)*reference.size() + (col-1)] + 1;
                else
                    match = swarray[(row-1)*reference.size() + (col-1)] - 1;
                if (match > bestScore)
                {
                    bestScore = match;
                    bestSource = 1;
                }
            }
            if (row == 0 || col == 0)
            {
                if (query[row] == reference[col])
                    bestScore = 1;
            }
            // Deletion (move down)
            if (row > 0)
            {
                deletion = swarray[(row-1)*reference.size() + col] -1;
                if (deletion > bestScore)
                {
                    bestScore = deletion;
                    bestSource = 2;
                }
            }
            // Insertion (move right)
            if (col > 0)
            {
                insertion = swarray[row*reference.size() + col - 1] -1;
                if (insertion > bestScore)
                {
                    bestScore = insertion;
                    bestSource = 3;
                }
            }
            swarray[row*reference.size() + col] = bestScore;
            backpointers[row*reference.size() + col] = bestSource;
        }
    }
    gettimeofday(&t2, NULL);
    runtime += (t2.tv_sec -t1.tv_sec) * 1000.0;
    runtime += (t2.tv_usec - t1.tv_usec) / 1000.0;
    cout << "Core time: " << runtime << endl;
    /*
    for (int row = 0; row < query.size(); row++)
    {
        for (int col = 0; col < reference.size(); col++)
        {
          cout << swarray[row*reference.size() + col] << ", ";
        }
        cout << endl;
    }
    */

    // FIND THE END
    int bestScore = 0;
    int bestRow = 0;
    int bestCol = 0;
    for (unsigned int row = 0; row < query.size(); row++)
    {
        for (unsigned int col = 0; col < reference.size(); col++)
        {
            if (swarray[row*reference.size() + col] > bestScore)
            {
                bestScore = swarray[row*reference.size() + col];
                bestRow = row;
                bestCol = col;
            }
            //cout << swarray[row*reference.size() + col] << ", ";
        }
        //cout << endl;
    }
    // BACKTRACK
    unsigned int endCol = bestCol;
    while (bestScore > 0)
    {
        // END OF THE ROAD
        if (backpointers[bestRow*reference.size() + bestCol] == 0)
        {
            bestScore = 0;
            alignedQuery += query[bestRow];
        }
        // MATCH/MISMATCH
        else if (backpointers[bestRow*reference.size() + bestCol] == 1)
        {
            alignedQuery += query[bestRow];
            bestRow -= 1;
            bestCol -= 1;
            bestScore = swarray[bestRow * reference.size() + bestCol];
        }
        // DELETION (move down)
        else if (backpointers[bestRow*reference.size() + bestCol] == 2)
        {
            // XXX this will remove something from the query
            bestRow -= 1;
            bestScore = swarray[bestRow * reference.size() + bestCol];
        }
        // INSERTION (move right)
        else if (backpointers[bestRow*reference.size() + bestCol] == 3)
        {
            alignedQuery += "-";
            bestCol -= 1;
            bestScore = swarray[bestRow * reference.size() + bestCol];
        }
    }
    while (bestCol > 0)
    {
        alignedQuery += "-";
        bestCol -= 1;
    }
    for (unsigned int i = 0; i < alignedQuery.size()/2; i++)
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

string* SWAligner::get_aligned()
{
    farm_queries();
    return alignment;
}

