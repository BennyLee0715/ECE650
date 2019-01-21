//First Fit malloc/free
void *ff_malloc(size_t size);
void ff_free(void *ptr);

//Best Fit malloc/free
void *bf_malloc(size_t size);
void bf_free(void *p

typedef struct BlockInfo{
    size_t size;
    struct BlockInfo * next;
    struct BlockInfo * prev;
    int isFree;
} block_t;
             
// Customized functions
void _free(void *ptr);

BlockInfo* head_block = NULL; // linked list head