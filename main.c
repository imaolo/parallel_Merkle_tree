// -lcrpyto -fopenmp -lm
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <openssl/sha.h>
#include <omp.h>
#include "merkle_tree.h"

typedef char BYTE ;

void parallel_merkle_root(BYTE **, merkle_tree*, BYTE *,size_t);
int Print(unsigned char *,size_t);
void print_tree(merkle_tree *);
void printArguments();
int checkArguments(int,char**);



//merkle tree test
#define FILE_NAME "results.csv"

// #define TREE_HEIGHT 5
// #define BLOCK_SIZE 100000000
// #define DATA_BLOCKS 16 


int main(int argc, char **argv)
{
    if (!checkArguments(argc,argv))
        return 0;

    printf("running...\n");
    const size_t TREE_HEIGHT  = atoi(argv[2]);
    const size_t DATA_BLOCKS  = pow(2,TREE_HEIGHT-1);
    size_t temp               = atof(argv[1]) * pow(10,6);
    const size_t MESSAGE_SIZE = temp%DATA_BLOCKS == 0 ? temp : temp + (temp%DATA_BLOCKS);
    const size_t BLOCK_SIZE   = MESSAGE_SIZE/DATA_BLOCKS;
    merkle_tree mt_a          = {0, TREE_HEIGHT, SHA_DIGEST_LENGTH, BLOCK_SIZE, DATA_BLOCKS, SHA1, NULL};
    merkle_tree mt_b          = {0, TREE_HEIGHT, SHA_DIGEST_LENGTH, BLOCK_SIZE, DATA_BLOCKS, SHA1, NULL};

    BYTE result[mt_a.hash_size];
    BYTE *message = calloc(BLOCK_SIZE*DATA_BLOCKS,sizeof(BYTE));
    BYTE **data   = calloc(DATA_BLOCKS,sizeof(BYTE *));
    for (int i = 0;i<DATA_BLOCKS;i++)
        data[i] = calloc(BLOCK_SIZE,sizeof(BYTE));




    //build tree mt_a with data
    double start = 0;

    start = omp_get_wtime();
    //build_tree(&mt_a, data);
    mt->hash_function(message,MESSAGE_SIZE,result);
    printf("Serial Time  : %f\n",omp_get_wtime()-start);
    start = omp_get_wtime();
    parallel_merkle_root(data,&mt_b,result, 1);
    printf("parallel Time: %f\n",omp_get_wtime()-start);



    BYTE passed = 1;
    for (int i = 0;i<mt_a.hash_size;i++){
        if (result[i] != mt_a.nodes[1].hash[i])
            passed = 0;
    }
    if (passed)
        printf("PASSED\n");
    else
        printf("FAILED\n");


    
    //free merkle tree objects
    for (int i = 0;i<DATA_BLOCKS;i++)
        free(data[i]);
    free(data);
    free(message);
    freeMerkleTree(&mt_a);
    return 0;
}

void parallel_merkle_root(BYTE **data, merkle_tree *mt, BYTE *result,size_t threads){

    merkle_tree left_tree = {
        0, 
        mt->tree_height-1,
        mt->hash_size,
        mt->data_block_size, 
        mt->data_blocks/2, 
        mt->hash_function, 
        NULL};
    merkle_tree right_tree = {
        0, 
        mt->tree_height-1,
        mt->hash_size,
        mt->data_block_size, 
        mt->data_blocks/2, 
        mt->hash_function, 
        NULL};
    //build in parallel
    #pragma omp parallel shared(left_tree,right_tree,data,mt)
    {
        if (omp_get_thread_num() == 0)
            build_tree(&left_tree,data);
        else
            build_tree(&right_tree,&data[mt->data_blocks/2]);
    } 

    unsigned char concat[mt->hash_size*2];
    for (int i=0;i<mt->hash_size;i++)
        concat[i] = left_tree.nodes[1].hash[i];
    for (int i=0;i<mt->hash_size;i++)
        concat[i+mt->hash_size] = right_tree.nodes[1].hash[i];
    mt->hash_function(concat,mt->hash_size*2,result);

    freeMerkleTree(&left_tree);
    freeMerkleTree(&right_tree);
    // for (int i = 0;i<mt->data_blocks;i++)
    //     free(data[i]);
    // free(data);
    return;

}

int Print(unsigned char *digest,size_t length){
    int i;
    if (!digest) {
        printf("<empty hash>\n");
        return -1;
    }
     for(i = 0; i< length; i++)
        printf("%02x", digest[i]);
     printf("\n");
     return 0;
}

void print_tree(merkle_tree *mt) {

    printf("--------------------------------\n");
    for(int i=1; i<=mt->n; i++)
        Print(mt->nodes[i].hash,mt->hash_size);
    printf("--------------------------------\n");
    return;
}

void printArguments(){
    printf("The arguments are: \n");
    printf("1. message size(MB)\n");
    printf("2. tree height\n");
    printf("3. tests\n");
}

int checkArguments(int argc, char **argv){
    if (argc != 4){
        printf("Please include the proper amount of arguments.\n");
        printArguments();
        return 0;
    }
    else if (atof(argv[1]) <= 0){
        printf("Please enter a float greater than zero for message size.\n");
        printArguments();
        return 0;
    }
    else if (atoi(argv[2]) <= 0){
        printf("Please enter a float greater than zero for tree height.\n");
        printArguments();
        return 0 ; 
    }
    else if (atoi(argv[3]) <= 0){
        printf("Please enter an integer greater than zero for number of tests.\n");
        printArguments();
        return 0;
    }
    return 1;
}
