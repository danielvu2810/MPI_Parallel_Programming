#include "mpi.h"
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>

using namespace std;

/* Global variables, Look at their usage in main() */

int* calculateTotalHistogram(int* histogram,int* histogramInfo){
    for(int i = 0; i < 257; i++){
        histogram[i] += histogramInfo[i];
    }
    
    return histogram;
}

int* countHistogram(int* histogramInfo,int* imageInfo,int chunkInfo){
    int temp;
    
    for(int i = 0; i < 257; i++) histogramInfo[i] = 0;
    
    histogramInfo[0] += 1;
        
    for(int i = 0; i < chunkInfo; i++){
        temp = histogramInfo[imageInfo[i] + 1];
        temp += 1;
        histogramInfo[imageInfo[i] + 1] = temp;
    }

    return histogramInfo;
}

int* updateVisited(int root, int* visited, int num_processes){
    
    MPI_Recv(visited,num_processes,MPI_INT,MPI_ANY_SOURCE,1,MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    visited[root] = 1;
    
    for(int i = 0; i < num_processes;i ++){
        MPI_Send(visited,num_processes,MPI_INT,i,1,MPI_COMM_WORLD);
    }
    
    return visited;
}

/* **************** Change the function below if you need to ***************** */

int main(int argc, char* argv[])
{
    int processId, num_processes;
    int image_height, image_width, image_maxShades;
    int* inputImage;
    int* adjMatrix;
    int* chunkSizes;
    int* visited;
    
    int* newhistogram = (int*) malloc(sizeof(int) * 257);
    for(int i = 0; i < 257;i++){
        newhistogram[i] = 0;
    }
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
    
    if(argc != 4)
    {
        if(processId == 1)
        {
            std::cout << "ERROR: Incorrect number of arguments. Format is: <Input image filename> <Path To text file> <Output File Name>" << std::endl;
        }
        MPI_Finalize();
        return 0;
    }
 
    if (processId == 1){
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
        
        chunkSizes = new int[num_processes];
        
        int total_pixels = image_height*image_width;
        int size = floor(total_pixels*1.0/num_processes);
        
        for(int i = 0; i < num_processes; i++){
            if(total_pixels > 0){
                if(total_pixels >= size*2){
                    chunkSizes[i] = size;
                    total_pixels -= size;
                }
                else
                {
                    chunkSizes[i] = total_pixels;
                    total_pixels = 0;
                }
            }
        }
        
        adjMatrix = new int[num_processes*num_processes];

        int temp;
        std::string cell;
        std::ifstream file2(argv[2]);
        if(!file2.is_open())
        {
            std::cout << "ERROR: Could not open file " << argv[2] << std::endl;
            return 0;
        }
        else{
            int t = 0;
            while(std::getline(file2,workString))
            {
                std::stringstream stream(workString);
                                    
                while(std::getline(stream, cell, ' '))
                {
                    adjMatrix[t] = stoi(cell);
                    t++;
                }
            }
        }
        
    }
    
    int chunkInfo;
    visited = (int*) malloc(sizeof(int) * num_processes);
    
    int* histogram = (int*) malloc(sizeof(int) * 257);
    int* histogramInfo = (int*) malloc(sizeof(int) * 257);

    for(int i = 0; i < num_processes; i++){
        visited[i] = 0;
    }
    
    int root = -1;
    
    MPI_Scatter(chunkSizes,1,MPI_INT,&chunkInfo,1,MPI_INT,1,MPI_COMM_WORLD);
        
    int* imageInfo = (int*) malloc(sizeof(int) * chunkInfo);
    
    MPI_Scatter(inputImage,chunkInfo,MPI_INT,imageInfo,chunkInfo,MPI_INT,1,MPI_COMM_WORLD);

    int* matrixInfo = (int*) malloc(sizeof(int) * num_processes);

    MPI_Scatter(adjMatrix,num_processes,MPI_INT,matrixInfo,num_processes,MPI_INT,1,MPI_COMM_WORLD);
        
    //if this is root node
    if(processId == 1){
        visited[processId] = 1;
        
        for(int i = 0; i < 257; i++){
            histogram[i]=0;
        }
        
        for(int i = 0; i < num_processes; i++){ //send to all neighbor
            if(matrixInfo[i] == 1){
                root = processId;
                MPI_Send(&root,1,MPI_INT,i,0,MPI_COMM_WORLD);
            }
           
            MPI_Send(visited,num_processes,MPI_INT,i,1,MPI_COMM_WORLD);
        }
        
        // receive back from all neighbors and add its histogram
        for(int i = 0; i < num_processes; i++){
            if(i != 1){
                MPI_Recv(histogramInfo,257,MPI_INT,MPI_ANY_SOURCE,3+i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
            else{
                histogramInfo = countHistogram(newhistogram,imageInfo,chunkInfo);
            }
            
            histogram = calculateTotalHistogram(histogram,histogramInfo);
        }
        
        std::ofstream ofile(argv[3]);
        
        if (ofile.is_open()) {
            for(int i = 1; i< 257; i++){
                ofile <<histogram[i]<<endl;
            }
        }
        else {
            std::cout << "ERROR: Could not open output file " << argv[3] << std::endl;
            return 0;
        }
    }
    
    else {
        int root;
        
        MPI_Recv(&root,1,MPI_INT,MPI_ANY_SOURCE,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if(root != -1){
            int has_neighbors = 0;
            
            visited = updateVisited(root,visited, num_processes);
            
            for(int i = 0; i < num_processes; i++){
                
                //traverse to next node if has neighbor and not visited
                if(matrixInfo[i] == 1 && visited[i] != 1){
                    has_neighbors = 1;
                    MPI_Send(&processId,1,MPI_INT,i,0,MPI_COMM_WORLD);
                }
            }
                            
            // at the end, count the histogram and send back to the parent node
            histogramInfo = countHistogram(histogramInfo,imageInfo,chunkInfo);
            MPI_Send(histogramInfo,257,MPI_INT,1,3+processId,MPI_COMM_WORLD);
        }
    }
    
    MPI_Finalize();
    return 0;
}
