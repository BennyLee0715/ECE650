#include <stdio.h>
#include <unistd.h>
// First Fit malloc/free
void *ff_malloc(size_t size);
void ff_free(void *ptr);

// Best Fit malloc/free
void *bf_malloc(size_t size);
void bf_free(void *ptr);

// Performance study
unsigned long get_data_segment_size();
long long get_data_segment_free_space_size(); // in bytes

typedef struct BlockInfo {
  size_t size;
  int isFree;
  struct BlockInfo *next_phys;
  struct BlockInfo *prev_phys;
  struct BlockInfo *next_list;
  struct BlockInfo *prev_list;
} block_t;

typedef block_t *(*FunType)(size_t);

// Customized functions

/*
 * 1. check valid(NULL)
 * 2. remove from free list
 * 3. merge operation
 */
void _free(void *ptr);

/*
 * 1. add to the front
 * 2. modify necessary pointers
 */
void free_list_add(block_t *block);

/*
 * 1. modify the isFree flag
 * 2. modify the adjacent block pointers
 */
void free_list_remove(block_t *block);

/*
 * 1. check whether it is free
 * 2. merge the size
 * 3. update pointers
 */
void free_list_merge(block_t *block);

/*
 * 1. Modify the size of two blocks
 * 2. Modify phisical adjacent pointers
 * 3. Return address of split part.
 */
block_t *split(block_t *block, size_t size);

/*
 * 1. call sbrk for memory allocation
 * TODO: further improvements with specific allocation unit
 */
block_t *request_memory(size_t size);

/*
 * 1. iterations to find one
 */
block_t *find_ff(size_t size);
block_t *find_bf(size_t size);

/*
 * 1. check free list
 * 2. judge perfect block
 */
void *_malloc(size_t size, FunType fp);

void phys_list_remove(block_t *block);

block_t *head_block = NULL; // linked list head
block_t *tail_block = NULL; // physical list tail
unsigned long data_segment_size = 0;
long long data_segment_free_space_size = 0;
