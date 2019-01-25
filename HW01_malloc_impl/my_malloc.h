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

// Store Meta Information of a block
typedef struct BlockInfo {
  size_t size;
  int isFree;
  struct BlockInfo *next_phys;
  struct BlockInfo *prev_phys;
  struct BlockInfo *next_list;
  struct BlockInfo *prev_list;
} block_t;

typedef block_t *(*FunType)(size_t);

#ifndef ALLOC_UNIT
#define ALLOC_UNIT 3 * sysconf(_SC_PAGESIZE)
#endif

// Customized functions

// For two policies, we use the same free method here
void _free(void *ptr);

// Add a block to the front of the list
void free_list_add_front(block_t *block);

// Remove a block from list
void free_list_remove(block_t *block);

// Megre adjacent free blocks around 'block'
void free_list_merge(block_t *block);

// Split block into two, the meta of block still accounts for
// free block, the rest part is regarded as malloced one.
block_t *split(block_t *block, size_t size);

// call sbrk for memory allocation
block_t *request_memory(size_t size);

// Different policies to find block
block_t *find_ff(size_t size);
block_t *find_bf(size_t size);

// malloc space according to size and policy
void *_malloc(size_t size, FunType fp);

// Merge block->next_phys into block, and form a greater one.
void phys_list_merge_next(block_t *block);

block_t *head_block = NULL; // linked list head
block_t *tail_block = NULL; // physical list tail

unsigned long data_segment_size = 0;
long long data_segment_free_space_size = 0;
