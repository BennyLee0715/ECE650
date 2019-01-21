#include <stdio.h>
#include <unistd.h>

//First Fit malloc/free
void *ff_malloc(size_t size);
void ff_free(void *ptr);

//Best Fit malloc/free
void *bf_malloc(size_t size);
void bf_free(void *ptr);

//Performance study
unsigned long get_data_segment_size();
unsigned long get_data_segment_free_space_size(); //in bytes


typedef struct BlockInfo{
    size_t size;
    struct BlockInfo * next;
    struct BlockInfo * prev;
    int isFree;
} block_t;

// Customized functions
void _free(void *ptr);

block_t* head_block = NULL; // linked list head
unsigned long data_segment_size = 0;
unsigned long data_segment_free_space_size = 0;