#include "my_malloc.h"

// First Fit malloc/free
void *ff_malloc(size_t size) { return _malloc(size, find_ff); }
void ff_free(void *ptr) { _free(ptr); }

// Best Fit malloc/free
void *bf_malloc(size_t size) { return _malloc(size, find_bf); }
void bf_free(void *ptr) { _free(ptr); }

// Performance study
unsigned long get_data_segment_size() { return data_segment_size; }
long long get_data_segment_free_space_size() {
  return data_segment_free_space_size;
}

/*
 * 1. check valid(NULL)
 * 2. remove from free list
 * 3. merge operation
 */
void _free(void *ptr) {
  if (ptr == NULL)
    return;
  block_t *block = ptr - sizeof(block_t);
  data_segment_free_space_size += block->size + sizeof(block_t);
  free_list_add(block);
  free_list_merge(block);
}

/*
 * 1. add to the front
 * 2. modify necessary pointers
 */
void free_list_add(block_t *block) {
  
  block->isFree = 1;
  block->next_list = head_block;
  if (head_block)
    head_block->prev_list = block;
  head_block = block;
}

/*
 * 1. check whether it is free
 * 2. merge the size
 * 3. update pointers
 */
void free_list_merge(block_t *block) {
  
  block_t *prev_phys = block->prev_phys;
  block_t *next_phys = block->next_phys;
  

  if (next_phys && next_phys->isFree) {
    phys_list_remove(block);
    free_list_remove(next_phys);
  }

  if (prev_phys && prev_phys->isFree) {
    phys_list_remove(prev_phys);
    free_list_remove(block);
  }
}

// will merge block->next to itself
void phys_list_remove(block_t *block) {
  block->size += sizeof(block_t) + block->next_phys->size;
  if (block->next_phys != tail_block) {
    block->next_phys->next_phys->prev_phys = block;
  } else {
    tail_block = block;
  }
  block->next_phys = block->next_phys->next_phys;
}

/*
 * 1. modify the isFree flag
 * 2. modify the adjacent block pointers
 */
void free_list_remove(block_t *block) {
  block_t *prev_list = block->prev_list;
  block_t *next_list = block->next_list;
  block->prev_list = NULL;
  block->next_list = NULL;
  block->isFree = 0;
  if (prev_list)
    prev_list->next_list = next_list;
  else
    head_block = next_list;

  if (next_list)
    next_list->prev_list = prev_list;
}

/*
 * 1. Modify the size of two blocks
 * 2. Modify phisical adjacent pointers
 * 3. Return address of split part.
 */
block_t *split(block_t *block, size_t size) {
  block_t *new_block = (void *)block + sizeof(block_t) +
                       (block->size - (sizeof(block_t) + size));
  new_block->isFree = 0;
  new_block->size = size;
  new_block->prev_list = NULL;
  new_block->next_list = NULL;
  new_block->prev_phys = block;
  new_block->next_phys = block->next_phys;


  if (block->next_phys)
    block->next_phys->prev_phys = new_block;
  block->next_phys = new_block;
  block->size -= sizeof(block_t) + size;
  if (new_block->next_phys == NULL)
    tail_block = new_block;
  return new_block;
}

/*
 * 1. call sbrk for memory allocation
 * TODO: further improvements with specific allocation unit
 */
block_t *request_memory(size_t size) {
  size_t alloc_size = size + sizeof(block_t);
  void *ptr = sbrk(alloc_size);
  if (ptr == (void *)-1) {
    return NULL;
  }
  data_segment_size += alloc_size;
  block_t *block = ptr;
  block->size = size;
  block->isFree = 0;
  block->prev_list = NULL;
  block->next_list = NULL;
  block->prev_phys = tail_block;
  block->next_phys = NULL;
  if (tail_block) {
    tail_block->next_phys = block;
  }
  tail_block = block;
  return block;
}

/*
 * 1. iterations to find one
 */
block_t *find_ff(size_t size) {
  block_t *curr = head_block;
  while (curr) {
    if (curr->size >= size) {
      return curr;
    }
    curr = curr->next_list;
  }
  return NULL;
}
block_t *find_bf(size_t size) {
  block_t *optimal = NULL;
  block_t *curr = head_block;
  while (curr) {
    if (curr->size > size + sizeof(block_t)) {
      if (optimal == NULL || curr->size < optimal->size) {
        optimal = curr;
      }
    } else if (curr->size >= size) {
      return curr;
    }
    curr = curr->next_list;
  }
  return optimal;
}

/*
 * 1. check free list
 * 2. judge perfect block
 */
void *_malloc(size_t size, FunType fp) {
  if (head_block == NULL) {
    block_t *block = request_memory(size);
    return block == NULL ? NULL : block + 1;
  } else {
    block_t *block = (*fp)(size);
    if (block == NULL) {
      block = request_memory(size);
      return block == NULL ? NULL : (block + 1);
    } else if (block->size <= size + sizeof(block_t)) {
      data_segment_free_space_size -= block->size + sizeof(block_t);
      free_list_remove(block);
      return block + 1;
    } else {
      data_segment_free_space_size -= size + sizeof(block_t);
      block_t *new_block = split(block, size);
      return new_block + 1;
    }
  }
}