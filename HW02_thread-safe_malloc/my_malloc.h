#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);

// Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

// Store Meta Information of a block
typedef struct BlockInfo {
  size_t size;
  struct BlockInfo *next_list;
  struct BlockInfo *prev_list;
} block_t;

// function type for sbrk
typedef void *(*FunType)(intptr_t);

#ifndef ALLOC_UNIT
#define ALLOC_UNIT 3 * sysconf(_SC_PAGESIZE)
#endif

// Customized functions

// For two policies, we use the same free method here
void _free(void *ptr, block_t **head_block, block_t **tail_block);

// Add a block to the front of the list
void free_list_insert(block_t *block, block_t **head_block,
                      block_t **tail_block);

// Remove a block from list
void free_list_remove(block_t *block, block_t **head_block,
                      block_t **tail_block);

// Megre adjacent free blocks around 'block'
void free_list_merge(block_t *block, block_t **head_block,
                     block_t **tail_block);

// Split block into two, the meta of block still accounts for
// free block, the rest part is regarded as malloced one.
block_t *split(block_t *block, size_t size);

// call sbrk for memory allocation
block_t *request_memory(size_t size, FunType fp, block_t **head_block,
                        block_t **tail_block);

// sbrk with lock
void *sbrk_lock(intptr_t size);

// policy to find block
block_t *find_bf(size_t size, block_t **head_block);

// malloc space according to size and policy
void *_malloc(size_t size, FunType fp, block_t **head_block,
              block_t **tail_block);

block_t *head_block_lock = NULL; // linked list head
block_t *tail_block_lock = NULL; // linked list tail

__thread block_t *head_block_tls = NULL; // linked list head
__thread block_t *tail_block_tls = NULL; // linked list tail

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// init block
void set_block(block_t *block, size_t size, block_t *prev_list,
               block_t *next_list);