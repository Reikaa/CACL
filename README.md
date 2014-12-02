CACL
====

Codon Aligner in OpenCL, currently for aligning queries to a reference.

Current Status
==============
Basic alignment (SW) is implemented, with a first swing at local memory optimization.

Compilation
===========
Use "make all" to compile both cpu and OpenCL (ocl) versions.

Inputs and Outputs
==================
- Input: a fasta file with the reference first
- Output: (temporary) execution times. Will be the actual alignment to a file soon.

Usage
=====
./swocl in_file.fasta out_file.fasta

Future Steps
============
- Integer character encodings
- Useful emissions
- Character distance matrices of any size
- Affine gap penalties
- Coalesced global memory accessions
- Greater occupancy
- Branch minimalization
- Codon pattern alignment
- OpenCL device selection
