typedef unsigned char* (*Hash_Function) (const unsigned char *, size_t  , unsigned char *);

typedef struct { 
    char *hash;
    char *data;
} merkle_tree_node;

typedef struct {
    size_t n;
    size_t tree_height;
    size_t hash_size;
    size_t data_block_size;
    size_t data_blocks;
    Hash_Function hash_function;
    merkle_tree_node *nodes;
} merkle_tree;

int build_tree(merkle_tree *, char **);
int tree_cmp(merkle_tree *, merkle_tree *, size_t );
int set_tree_data(merkle_tree *, size_t , char *);
void freeMerkleTree(merkle_tree *);
