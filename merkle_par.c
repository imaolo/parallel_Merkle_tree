#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include "merkle_par.h"

static int hash_node(merkle_tree *mt, size_t i);

int merke_root(merkle_tree *mt,BYTE *message, BYTE* result){
    if (mt->data_blocks > (1 << (mt->tree_height - 1)))
        return -1;

    BYTE **data = calloc(mt->data_block_size*mt->data_blocks,sizeof(BYTE));
    for (int i = 0;i<mt->data_blocks;i++)
        memcpy(data[i],message[i*mt->data_block_size],mt->data_block_size);

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

    build_tree(&left_tree,data);
    build_tree(&right_tree,&data[mt->data_blocks/2]);
    unsigned char message[mt->hash_size*2];
    for (int i=0;i<mt->hash_size;i++)
        message[i] = left_tree.nodes[1].hash[i];
    for (int i=0;i<mt->hash_size;i++)
        message[i+mt->hash_size] = right_tree.nodes[1].hash[i];
    mt->hash_function(message,mt->hash_size*2,result);

    free_merkle_tree(&right_tree);
    free_merkle_tree(&left_tree);
    free(data);
    return 0;
}

int build_tree(merkle_tree *mt, char *data[]) {

    if (mt->data_blocks > (1 << (mt->tree_height - 1)))
        return -1;
    int i, leaf_start;
    leaf_start = (1 << (mt->tree_height - 1)); //computes the amount of leaves
    mt->n = leaf_start + mt->data_blocks - 1; //computes the amount of nodes
    mt->nodes = (merkle_tree_node *)malloc(sizeof(merkle_tree_node) * (mt->n + 1));
    for (i = leaf_start; i <= mt->n; i++) {
        mt->nodes[i].data = data[i-leaf_start];
        mt->nodes[i].hash = NULL;
        if (hash_node(mt, i) == -1)
            return -1;
    }
    for (i = leaf_start - 1; i > 0; i--) {
        mt->nodes[i].hash = NULL;
        if (hash_node(mt, i) == -1)
            return -1;
    }
    return 0;
}

int build_tree_parallel(merkle_tree *mt, char *data[]) {

    if (mt->data_blocks > (1 << (mt->tree_height - 1)))
        return -1;
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
    build_tree(&left_tree,data);
    build_tree(&right_tree,&data[mt->data_blocks/2]);
    // printf("-----------------parallel--------------------\n");
    // printf("-----------------left--------------------\n");
    // print_tree(&left_tree);
    // printf("-----------------right--------------------\n");
    // print_tree(&right_tree);

    unsigned char result[mt->hash_size];
    unsigned char message[mt->hash_size*2];
    for (int i=0;i<mt->hash_size;i++)
        message[i] = left_tree.nodes[1].hash[i];
    for (int i=0;i<mt->hash_size;i++)
        message[i+mt->hash_size] = right_tree.nodes[1].hash[i];

    printf("message test: \n");
    printDig(message,mt->hash_size);
    printDig(&message[mt->hash_size],mt->hash_size);
    mt->hash_function(message,mt->hash_size,result);
    printf("top left: \n");
    printDig(left_tree.nodes[1].hash,mt->hash_size);
    printf("top right: \n");
    printDig(right_tree.nodes[1].hash,mt->hash_size);
    printf("-----------------Paralllel-----------------------\n");
    printDig(result,mt->hash_size);
    printf("-------------------------------------------\n");
    return 0;
}

void freeMerkleTree(merkle_tree *mt) {

    int i;
    if (!mt)
        return;
    if (mt->nodes) {
        for (i=1; i<=mt->n; i++)
            if(mt->nodes[i].hash)
                free(mt->nodes[i].hash);
        free(mt->nodes);
    }
    return;
}

static int hash_node(merkle_tree *mt, size_t i) {

    if (i > (1<<mt->tree_height)-1)
        return -1;
    if (i < (1<<mt->tree_height-1)){
        if (2*i+1 <= mt->n && mt->nodes[2*i].hash && mt->nodes[2*i+1].hash) {
            char *buffer = (char *)malloc(sizeof(char *) * (2 * mt->hash_size + 1));
            memcpy(buffer, mt->nodes[2*i].hash, mt->hash_size);
            memcpy(buffer+mt->hash_size, mt->nodes[2*i+1].hash, mt->hash_size);
            if (!mt->nodes[i].hash)
                mt->nodes[i].hash = (char *)malloc(sizeof(char *) * mt->hash_size);
            mt->hash_function(buffer, 2*mt->hash_size, mt->nodes[i].hash);
            free(buffer);
        }
        else if (2*i <= mt->n && mt->nodes[2*i].hash) {
            if (!mt->nodes[i].hash)
                mt->nodes[i].hash = (char *)malloc(sizeof(char *) * mt->hash_size);
            memcpy(mt->nodes[i].hash, mt->nodes[2*i].hash, mt->hash_size);
        }
    }
    else {
        if (mt->nodes[i].data) {
            if (!mt->nodes[i].hash)
                mt->nodes[i].hash = (char *)malloc(sizeof(char *) * mt->hash_size);
            mt->hash_function(mt->nodes[i].data, mt->data_block_size, mt->nodes[i].hash);
        }
        else
            return -1;
    }
    return 0;
}





//merkle tree test
#define TREE_HEIGHT 4 
#define BLOCK_SIZE 2
#define DATA_BLOCKS 8 


// int main()
// {
//     int i;
//     char *data[DATA_BLOCKS], *data_copy[DATA_BLOCKS], buffer[BLOCK_SIZE];
    
//     // make sure TREE_HEIGHT fits DATA_BLOCKS...
//     // BLOCK_SIZE & hash_size, hash_function also needed init
//     merkle_tree mt = {0, TREE_HEIGHT, SHA_DIGEST_LENGTH, BLOCK_SIZE, DATA_BLOCKS, SHA1, NULL};

//     for (i=0; i<BLOCK_SIZE; i++)
//         buffer[i] = 'A';
//     for (i=0; i<DATA_BLOCKS; i++) {
//         data[i] = (char *)malloc(sizeof(char) * BLOCK_SIZE);
//         memcpy(data[i], buffer, BLOCK_SIZE);
//     }

//     //build tree mt_a with data
//     build_tree(&mt, data);
//     build_tree_parallel(&mt, data);

//     printf("-------------Serial--------------------------\n");
//     printDig(mt.nodes[1].hash,mt.hash_size);
//     printf("-------------------------------------------\n\n\n");

//     print_tree(&mt);    

//     freeMerkleTree(&mt);
//     return 0;
// }
