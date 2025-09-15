# Dancing Links in C++
C++ implementation of the Dancing links algorithm, also known as Knuth's algorithm X, discovered by Donald Knuth.

Solves the exact cover problem in the form of a sparse matrix where the first line is the number of columns, see example.txt.

Note: Run time is proportional to the number of solutions, so the program struggles to solve relatively small matrices if the number of solutions is very large. It is therefore not suitable for solving tiling problems like the one found [here](https://projecteuler.net/problem=161).