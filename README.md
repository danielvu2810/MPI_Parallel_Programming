# MPI_Parallel_Programming

*Using MPI to solve Parallel processing for image*

## PART A:
An image histogram using Tarry’s algorithm
You are given two matrices, D and A. D is a grayscale image with 8-bit pixels. A is a matrix describing the DS connectivity (an adjacency bit matrix). Row i in the matrix A describes nodes that node i is connected to (a 1 in position A[i,j] means node j have a direct, bidirectional link). A node can send messages only to its neighbors (as per the adjacency matrix). Assume that otherwise it does not know how many nodes are in the system. 

### Algorithm Tarry:
if this is the root node
  send token to a neighbor

Every node waits to receive a message
Once the node has received a token, it adds it's data to the token
set the parent to the node which sent the token we just received
for each  neighbor in the set of neighbors:
  if neighbor has not been visited and it is not the parent of the current node:
    send the token to that neighbor
    receive a token from that neighbor
once we have iterated through all neighbors, send the token back to the parent
if the current node is a root node, wait for the token to arrive and then output the token

## PART B:
Using different methods: MPI_Reduce and Righ Topology to count the word frequency

