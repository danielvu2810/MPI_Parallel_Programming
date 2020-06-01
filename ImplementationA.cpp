//#include "mpi.h"
//#include <omp.h>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

/*
 /usr/local/opt/llvm/bin/clang++ -fopenmp -L/usr/local/opt/llvm/lib ImplementationA.cpp -o PartA
 
 ./PartA tc1.pbm tc1_test_a2_25.pbm 14 a2
 */

/* Global variables, Look at their usage in main() */

int matrix_length;
int adj_matrix[1000][1000];
int outputImage[1000][1000];

/* **************** Change the function below if you need to ***************** */

int main(int argc, char* argv[])
{
    int processId, num_processes;
    int image_height, image_width, image_maxShades;
    int* inputImage;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
    
    if(argc != 4)
    {
        if(processId == 2)
        {
            std::cout << "ERROR: Incorrect number of arguments. Format is: <Input image filename> <Path To text file> <Output File Name>" << std::endl;
        }
        MPI_Finalize();

        return 0;
    }
 
    if (processId == 2){
        std::ifstream file(argv[1]);
        if(!file.is_open())
        {
            std::cout << "ERROR: Could not open file " << argv[1] << std::endl;
            return 0;
        }
        
        std::cout << "Detect edges in " << argv[1] << " using " << num_processes << " processes" << std::endl;
        
        std::string workString;
        
        /* Remove comments '#' and check image format */
        while(std::getline(file,workString))
        {
            if( workString.at(0) != '#' ){
                if( workString.at(1) != '2' ){
                    std::cout << "Input image is not a valid PGM image" << std::endl;
                    return 0;
                } else {
                    break;
                }
            } else {
                continue;
            }
        }
        
        /* Check image size */
        while(std::getline(file,workString))
        {
            if( workString.at(0) != '#' ){
                std::stringstream stream(workString);
                int n;
                stream >> n;
                image_width = n;
                stream >> n;
                image_height = n;
                break;
            } else {
                continue;
            }
        }

        /* Check image max shades */
        while(std::getline(file,workString))
        {
            if( workString.at(0) != '#' ){
                std::stringstream stream(workString);
                stream >> image_maxShades;
                break;
            } else {
                continue;
            }
        }
        
        inputImage = new int[image_height*image_width];

        int pixel_val;
        for( int i = 0; i < image_height; i++ )
        {
            if( std::getline(file,workString) && workString.at(0) != '#' ){
                std::stringstream stream(workString);
                for( int j = 0; j < image_width; j++ ){
                    if( !stream )
                        break;
                    stream >> pixel_val;
                    inputImage[i*image_width+j] = pixel_val;
                }
            } else {
                continue;
            }
        }
        
        std::ifstream file(argv[2]);
        if(!file.is_open())
        {
            std::cout << "ERROR: Could not open file " << argv[2] << std::endl;
            return 0;
        }
        else{
            
        }
    }
    
    
    return 0;
}
