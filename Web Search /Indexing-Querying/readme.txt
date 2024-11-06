Project Descriptions - 


1. Inverted Index - 

Basic Search Engine (Inverted Index)

Introduction

This is part of a larger idea to try and create a web search engine. It provides a program to create an inverted index on the MS MARCO Dataset and perform query processing over it.

It has been implemented using efficient data processing, optimized sorting, and compression(this will most likely be reworked for future iterations), for fast and accurate query processing.

The following has been implemented as part of the query processing module -
•⁠ ⁠Ranking using the BM25 algorithm to return Top N results
•⁠ ⁠Searching Methods - Conjunctive and Disjunctive

Requirements

1. Operating System: Linux or Windows with a C++ compiler (GCC 7.5 or higher recommended).
2. Libraries: Standard C++ library, optional multithreading library for large datasets.
3. Dataset: Directory of text files for indexing.
4. File Permissions: Ensure read and write permissions for input, intermediate, and output directories.
5. Boost - To install, please run the command "brew install boost". If brew is not there, then please install homebrew
6. Require the collection.tsv (Passage dataset from MS MARCO datasets) 


Execution

1.⁠ Ensure you have the directories/paths provided properly
2.⁠ ⁠Compile and Execute: Navigate to the project directory and compile the files:

Compilation and Execution Order -
(You can run Query Processing - qpcdf as standalone once all other files are generated and present from the 2nd time)

1.g++ -std=c++20 -O2 -Wall inverted_index.cpp -o inverted_index

./inverted_index

2.g++ -std=c++20 -O2 -Wall new_sort.cpp -o new_sort

./new_sort

3.g++ -std=c++20 -O2 -Wall doc_file.cpp -o doc_file

./doc_file

4 .g++ -std=c++20 -O2 -Wall -I/opt/homebrew/include -L/opt/homebrew/lib -lboost_serialization compression.cpp -o compression

./compression

5.g++ -std=c++20 -O2 -Wall -I/opt/homebrew/include -L/opt/homebrew/lib -lboost_serialization qpcdf.cpp decompress.cpp -o qpcdf

./qpcdf
