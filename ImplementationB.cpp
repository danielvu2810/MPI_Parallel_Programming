#include "mpi.h"
#include <algorithm>
#include <functional>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>
using namespace std;
const static int ARRAY_SIZE = 130000;
using Words = char[16];
using Lines = char[ARRAY_SIZE][16];

/************* Feel free to change code below (even main() function), add functions, etc.. But do not change CL arguments *************/

struct letter_only: std::ctype<char> 
{
    letter_only(): std::ctype<char>(get_table()) {}

    static std::ctype_base::mask const* get_table()
    {
        static std::vector<std::ctype_base::mask> 
            rc(std::ctype<char>::table_size,std::ctype_base::space);

        std::fill(&rc['A'], &rc['z'+1], std::ctype_base::alpha);
        return &rc[0];
    }
};

int countFrequency(Words* words, int size, std::string target){
    int freq = 0;
    for (int i = 0; i < size; ++i){
        if(words[i]){
            if( !target.compare(words[i]) )
                freq++;
        }
    }
    return freq;
}

void DoOutput(std::string word, int result)
{
    std::cout << "Word Frequency: " << word << " -> " << result << std::endl;
}

int main(int argc, char* argv[])
{
    int processId;
    int numberOfProcesses;
    int *to_return = NULL;
    double start_time, end_time;
    int numLines = 0;
    int totalFreqs = 0;
 
    // Setup MPI
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &processId);
    MPI_Comm_size( MPI_COMM_WORLD, &numberOfProcesses);
 
    // Two arguments, the program name and the input file. The second should be the input file
    if(argc != 4)
    {
        if(processId == 0)
        {
            std::cout << "ERROR: Incorrect number of arguments. Format is: <path to search file> <search word> <b1/b2>" << std::endl;
        }
        MPI_Finalize();
        return 0;
    }
    
	std::string word = argv[2];
    
    Lines lines;
    if (processId == 0) {
        std::ifstream file;
		file.imbue(std::locale(std::locale(), new letter_only()));
		file.open(argv[1]);
		std::string workString;
		int i = 0;
		while(file >> workString){
			memset(lines[i], '\0', 16);
			memcpy(lines[i++], workString.c_str(), workString.length());
		}
        
        numLines = i;
    }
	
    char buf[(ARRAY_SIZE / numberOfProcesses) * 16];
    

    if(strcmp(argv[3], "b1") ==0 || strcmp(argv[3], "b2") == 0) {
        int* chunkSizes = NULL;
        
        int chunkInfo;
        
        if(processId == 0){
            chunkSizes = new int[numberOfProcesses];
            
            int words_per_chunk = ceil(numLines*1.0/numberOfProcesses);
            
            for(int i = 0; i < numberOfProcesses; i++){
                chunkSizes[i] = words_per_chunk;
            }
        }
        
        MPI_Scatter(chunkSizes,1,MPI_INT,&chunkInfo,1,MPI_INT,0,MPI_COMM_WORLD);
                            
        Words* chunkWords = new Words[chunkInfo];
        
        MPI_Scatter(&lines[0][0], chunkInfo*16, MPI_CHAR, &chunkWords[0][0],chunkInfo*16,MPI_CHAR,0, MPI_COMM_WORLD);
        
        int chunkWordsFreq;
        
        if (processId == 0) {
            start_time=MPI_Wtime();
        }
        
        if (strcmp(argv[3],"b1") == 0){
            chunkWordsFreq = countFrequency(chunkWords, chunkInfo, word);
            
            MPI_Reduce(&chunkWordsFreq,&totalFreqs,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
        }
        else
        {
            chunkWordsFreq = countFrequency(chunkWords, chunkInfo, word);

            if(numberOfProcesses == 1){
                totalFreqs = chunkWordsFreq;
            }
            else
            {
                if (processId == 0){
                    MPI_Send(&chunkWordsFreq, 1, MPI_INT, numberOfProcesses - 1, 0, MPI_COMM_WORLD);
                    MPI_Recv(&totalFreqs, 1, MPI_INT, 1, MPI_ANY_TAG, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                }
                else{
                    MPI_Recv(&totalFreqs, 1, MPI_INT, (processId+1)%(numberOfProcesses),MPI_ANY_TAG, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                    totalFreqs += chunkWordsFreq;
                    MPI_Send(&totalFreqs,1,MPI_INT,processId-1,0,MPI_COMM_WORLD);
                }
            }
        }
    }

    if(processId == 0)
    {
        DoOutput(word, totalFreqs);
        end_time=MPI_Wtime();
        std::cout << "time: " << ((double)end_time-start_time) << std::endl;
    }
 
    MPI_Finalize();
 
    return 0;
}
