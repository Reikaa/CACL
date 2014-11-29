STRINGIFY(

__kernel void basic_align(  __global char* reference,
                            __global char* query,
                            __global char* backpointers,
                            __global int* swarray,
                            __local int* scratch,
                            int refsize,
                            int querysize
                            )
{
    int row = get_local_id(0);
    int this_col = 0;
    scratch[row] = query[row];
    scratch[querysize + row] = 0;
    scratch[2 * querysize + row] = 0;
    for (int col = 0; col < refsize + querysize; col++)
    {
        //for (int row = 0; row < querysize; row++)
        if (col >= row && this_col < refsize)
        {
            //int start = 0;
            //int bestScore = start;
            int bestScore = 0;
            char bestSource = 0;

            // Possible moves:
            int match = 0;
            int insertion = 0;
            int deletion = 0;

            //if (query[row] == reference[this_col]) {
            if (scratch[row] == reference[this_col]) {
                match = 1;
            } else {
                match = -1;
            }
            //match += swarray[(row-1)*refsize + (this_col-1)];
            match += scratch[querysize + row-1];
            if (match > bestScore) {
                bestScore = match;
                bestSource = 1;
            }

            //deletion = swarray[(row-1)*refsize + this_col] -1;
            deletion = scratch[2*querysize + (row-1)] -1;
            if (deletion > bestScore) {
                bestScore = deletion;
                bestSource = 2;
            }
            //insertion = swarray[row*refsize + this_col - 1] -1;
            insertion = scratch[2*querysize + row] -1;
            if (insertion > bestScore) {
                bestScore = insertion;
                bestSource = 3;
            }
            scratch[querysize + row] = scratch[2*querysize + row];
            scratch[2*querysize + row] = bestScore;
            swarray[row*refsize + this_col] = bestScore;
            backpointers[row*refsize + this_col] = bestSource;
            this_col += 1;
        }
        barrier(CLK_GLOBAL_MEM_FENCE);
    }
};

__kernel void naive_align(  __global char* reference,
                            __global char* query,
                            __global char* backpointers,
                            __global int* swarray,
                            int refsize,
                            int querysize
                            )
{
    int row = get_local_id(0);
    int this_col = 0;
    for (int col = 0; col < refsize + querysize; col++)
    {
        //for (int row = 0; row < querysize; row++)
        if (col >= row && this_col < refsize)
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
            if (row > 0 && this_col > 0)
            {
                if (query[row] == reference[this_col])
                    match = swarray[(row-1)*refsize + (this_col-1)] + 1;
                else
                    match = swarray[(row-1)*refsize + (this_col-1)] - 1;
                if (match > bestScore)
                {
                    bestScore = match;
                    bestSource = 1;
                }
            }
            if (row == 0 || this_col == 0)
            {
                if (query[row] == reference[this_col])
                    bestScore = 1;
            }
            // Deletion (move down)
            if (row > 0)
            {
                deletion = swarray[(row-1)*refsize + this_col] -1;
                if (deletion > bestScore)
                {
                    bestScore = deletion;
                    bestSource = 2;
                }
            }
            // Insertion (move right)
            if (this_col > 0)
            {
                insertion = swarray[row*refsize + this_col - 1] -1;
                if (insertion > bestScore)
                {
                    bestScore = insertion;
                    bestSource = 3;
                }
            }
            swarray[row*refsize + this_col] = bestScore;
            backpointers[row*refsize + this_col] = bestSource;
            this_col += 1;
        }
        barrier(CLK_GLOBAL_MEM_FENCE);
    }
};
);
