#include "my_malloc.h"
#include "assert.h"

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

// For two policies, we use the same free method here
void _free(void *ptr) {
  if (ptr == NULL)
    return;
  block_t *block = ptr - sizeof(block_t);
  // data_segment_free_space_size += block->size + sizeof(block_t);
  free_list_add_front(block);
  free_list_merge(block);
}

// Add a block to the front of the list
void free_list_add_front(block_t *block) {
  assert(block && block->isFree == 0);
  data_segment_free_space_size += block->size + sizeof(block_t);
  block->isFree = 1;
  block->next_list = head_block;
  if (head_block)
    head_block->prev_list = block;
  head_block = block;
}

// Megre adjacent free blocks around 'block'
void free_list_merge(block_t *block) {
  assert(block);
  assert(block->isFree);

  block_t *prev_phys = block->prev_phys;
  block_t *next_phys = block->next_phys;
  assert(prev_phys == NULL || prev_phys->next_phys == block);

  // Merge next to current block if possible
  // physically connected
  if (next_phys && next_phys->isFree &&
      (void *)next_phys == (void *)block + sizeof(block_t) + block->size) {
    assert(block->isFree);
    assert(block->next_phys == next_phys);
    phys_list_merge_next(block);
    free_list_remove(next_phys);
  }

  // Merge current block to its previous one if possible
  // physically connected
  if (prev_phys && prev_phys->isFree &&
      (void *)block == (void *)prev_phys + sizeof(block_t) + prev_phys->size) {
    assert(block->isFree);
    assert(prev_phys->next_phys == block);
    phys_list_merge_next(prev_phys);
    free_list_remove(block);
  }
}

// will merge block->next to itself
void phys_list_merge_next(block_t *block) {
  // assert(block);
  // assert(block->next_phys);
  // assert(block->isFree);
  // assert(block->next_phys->isFree);
  // assert(block->next_phys == (void *)block + sizeof(block_t) + block->size);
  // assert(block->next_phys->next_phys == NULL ||
  /* block->next_phys->next_phys == (void *)block->next_phys + */
  /* block->next_phys->size + */
  /* sizeof(block_t)); */
  block->size += sizeof(block_t) + block->next_phys->size;
  if (block->next_phys !=
      tail_block) { // if block->next_phys is not the last one
    block->next_phys->next_phys->prev_phys = block;
  } else { // block->next_phys is the last one on heap
    tail_block = block;
  }
  block->next_phys = block->next_phys->next_phys;
  // assert(block->next_phys == NULL ||
  /* block->next_phys == (void *)block + sizeof(block_t) + block->size); */
}

// Remove a block from list
void free_list_remove(block_t *block) {
  // assert(block && head_block);

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

// Split block into two, the meta of block still accounts for
// free block, the rest part is regarded as malloced one.
block_t *split(block_t *block, size_t size) {
  // assert(block && block->size >= size + sizeof(block_t));

  // constructing the new block which is not free
  block_t *new_block = (void *)block + sizeof(block_t) +
                       (block->size - (sizeof(block_t) + size));
  set_block(new_block, size, 0, block, block->next_phys, NULL, NULL);

  // assert(new_block->next_phys == NULL ||
  /* new_block->next_phys == */
  /* (void *)new_block + sizeof(block_t) + new_block->size); */

  if (block->next_phys)
    block->next_phys->prev_phys = new_block;
  block->next_phys = new_block;
  block->size -= sizeof(block_t) + size;
  if (new_block->next_phys == NULL)
    tail_block = new_block;
  // assert(block->next_phys == (void *)block + sizeof(block_t) +
  // block->size);
  return new_block;
}

/*
 * 1. call sbrk for memory allocation
 */
block_t *request_memory(size_t size) {
  size_t alloc_size = size + 2 * sizeof(block_t) > ALLOC_UNIT
                          ? size + sizeof(block_t)
                          : ALLOC_UNIT;
  void *ptr = sbrk(alloc_size);
  if (ptr == (void *)-1) {
    return NULL;
  }
  // assert(tail_block == NULL ||
  /* ptr == (void *)tail_block + sizeof(block_t) + tail_block->size); */
  data_segment_size += alloc_size;
  block_t *block = ptr;
  set_block(block, size, 0, tail_block, NULL, NULL, NULL);
  if (tail_block) {
    tail_block->next_phys = block;
    // assert(block == (void *)tail_block + sizeof(block_t) +
    // tail_block->size);
  }
  if (alloc_size == ALLOC_UNIT) {
    block_t *free_block = ptr + sizeof(block_t) + size;
    set_block(free_block, alloc_size - (sizeof(block_t) * 2 + size), 0, block,
              NULL, NULL, NULL);
    block->next_phys = free_block;
    tail_block = free_block;
    free_list_add_front(free_block);
  } else {
    block->next_phys = NULL;
    tail_block = block;
  }
  return block;
}

// iterate to find the first-fit
block_t *find_ff(size_t size) {
  // assert(head_block);
  block_t *curr = head_block;
  while (curr) {
    // find the first fir here
    if (curr->size >= size) {
      return curr;
    }
    curr = curr->next_list;
  }
  return NULL;
}

// iterate to find the best fit
block_t *find_bf(size_t size) {
  // assert(head_block);
  block_t *optimal = NULL;
  block_t *curr = head_block;
  while (curr) {
    if (curr->size > size + sizeof(block_t)) {
      // find suitable one, but may not the best one.
      if (optimal == NULL || curr->size < optimal->size) {
        optimal = curr;
      }
    } else if (curr->size >= size) {
      // A perfect block found
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
    // No free block available
    block_t *block = request_memory(size);
    return block == NULL ? NULL : block + 1;
  } else {
    block_t *block = (*fp)(size);
    if (block == NULL) {
      // no suitable found in free list
      // request new space
      block = request_memory(size);
      return block == NULL ? NULL : (block + 1);
    } else if (block->size <= size + sizeof(block_t)) {
      // Perfect block found
      data_segment_free_space_size -= block->size + sizeof(block_t);
      free_list_remove(block);
      return block + 1;
    } else {
      // found a block which is large enough to become two.
      data_segment_free_space_size -= size + sizeof(block_t);
      block_t *new_block = split(block, size);
      return new_block + 1;
    }
  }
}

// Inplemented in HW01 **************************************************
// best fit policy adopted.
void *ts_malloc_lock(size_t size) {
  assert(pthread_mutex_lock(&lock) == 0);
  void *ptr = _malloc(size, find_bf);
  assert(pthread_mutex_unlock(&lock) == 0);
  return ptr;
}

void ts_free_lock(void *ptr) {
  assert(pthread_mutex_lock(&lock) == 0);
  _free(ptr);
  assert(pthread_mutex_unlock(&lock) == 0);
}

void *ts_malloc_nolock(size_t size) {
  void *ptr = NULL;
  if (head_block == NULL) {
    // No free block available
    block_t *block = request_memory_nolock(size);
    ptr = block == NULL ? NULL : block + 1;
  } else {
    block_t *block = find_bf(size);
    if (block == NULL) {
      // no suitable found in free list
      // request new space
      block = request_memory_nolock(size);
      ptr = block == NULL ? NULL : (block + 1);
    } else if (block->size <= size + sizeof(block_t)) {
      // Perfect block found
      data_segment_free_space_size -= block->size + sizeof(block_t);
      free_list_remove(block);
      ptr = block + 1;
    } else {
      // found a block which is large enough to become two.
      data_segment_free_space_size -= size + sizeof(block_t);
      block_t *new_block = split(block, size);
      ptr = new_block + 1;
    }
  }
  return ptr;
}
void ts_free_nolock(void *ptr) { _free(ptr); }

/*
 * 1. call sbrk for memory allocation
 */
block_t *request_memory_nolock(size_t size) {
  size_t alloc_size = size + 2 * sizeof(block_t) > ALLOC_UNIT
                          ? size + sizeof(block_t)
                          : ALLOC_UNIT;
  assert(pthread_mutex_lock(&lock) == 0);
  void *ptr = sbrk(alloc_size);
  assert(pthread_mutex_unlock(&lock) == 0);

  if (ptr == (void *)-1) {
    return NULL;
  }
  // assert(tail_block == NULL ||
  /* ptr == (void *)tail_block + sizeof(block_t) + tail_block->size); */
  data_segment_size += alloc_size;
  block_t *block = ptr;
  set_block(block, size, 0, tail_block, NULL, NULL, NULL);

  if (tail_block) {
    tail_block->next_phys = block;
    // assert(block == (void *)tail_block + sizeof(block_t) +
    // tail_block->size);
  }
  if (alloc_size == ALLOC_UNIT) {
    block_t *free_block = ptr + sizeof(block_t) + size;
    set_block(free_block, alloc_size - (sizeof(block_t) * 2 + size), 0, block,
              NULL, NULL, NULL);

    block->next_phys = free_block;
    tail_block = free_block;
    free_list_add_front(free_block);
  } else {
    block->next_phys = NULL;
    tail_block = block;
  }
  return block;
}

void set_block(block_t *block, size_t size, int isFree, block_t *prev_phys,
               block_t *next_phys, block_t *prev_list, block_t *next_list) {
  block->size = size;
  block->isFree = isFree;
  block->prev_phys = prev_phys;
  block->next_phys = next_phys;
  block->prev_list = prev_list;
  block->next_list = next_list;
}
