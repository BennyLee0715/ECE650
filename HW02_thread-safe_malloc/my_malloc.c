#include "my_malloc.h"
#include "assert.h"

// Thread Safe malloc/free: locking version
void *ts_malloc_lock(size_t size) {
  assert(pthread_mutex_lock(&lock) == 0);
  void *ptr = _malloc(size, sbrk, &head_block_lock, &tail_block_lock);
  assert(pthread_mutex_unlock(&lock) == 0);
  return ptr;
}

void ts_free_lock(void *ptr) {
  assert(pthread_mutex_lock(&lock) == 0);
  _free(ptr, &head_block_lock, &tail_block_lock);
  assert(pthread_mutex_unlock(&lock) == 0);
}

// Thread Safe malloc/free: non-locking version
void *ts_malloc_nolock(size_t size) {
  void *ptr = _malloc(size, sbrk_lock, &head_block_tls, &tail_block_tls);
  return ptr;
}

void ts_free_nolock(void *ptr) {
  _free(ptr, &head_block_tls, &tail_block_tls);
}

void _free(void *ptr, block_t **head_block, block_t **tail_block) {
  if (ptr == NULL) return;
  block_t *block = (block_t *)ptr - 1;
  if (block->isFree) return;
  free_list_add_front(block, head_block);
  free_list_merge(block, head_block, tail_block);
}

// Add a block to the front of the list
void free_list_add_front(block_t *block, block_t **head_block) {
  assert(block);
  block->isFree = 1;
  if (*head_block) {
    (*head_block)->prev_list = block;
  }
  block->next_list = *head_block;
  *head_block = block;
  block->prev_list = NULL; // should be redaudent.
}

// Remove a block from list
void free_list_remove(block_t *block, block_t **head_block) {
  assert(block);
  block->isFree = 0;
  if (block->prev_list) {
    block->prev_list->next_list = block->next_list;
  }
  else {
    *head_block = block->next_list;
  }
  if (block->next_list) {
    block->next_list->prev_list = block->prev_list;
  }
  block->next_list = NULL;
  block->prev_list = NULL;
}

// Megre adjacent free blocks around 'block'
void free_list_merge(block_t *block, block_t **head_block,
                     block_t **tail_block) {
  assert(block && block->isFree);
  if (block->next_phys && block->next_phys->isFree &&
      (void *)block + sizeof(block_t) + block->size == block->next_phys) {
    block_t *temp = block->next_phys;
    phys_list_merge_next(block, tail_block);
    free_list_remove(temp, head_block);
  }

  if (block->prev_phys && block->prev_phys->isFree &&
      (void *)block->prev_phys + sizeof(block_t) + block->prev_phys->size ==
          block) {
    phys_list_merge_next(block->prev_phys, tail_block);
    free_list_remove(block, head_block);
  }
}

// Split block into two, the meta of block still accounts for
// free block, the rest part is regarded as malloced one.
block_t *split(block_t *block, size_t size, block_t **tail_block) {
  assert(block && *tail_block && block->size >= sizeof(block_t) + size);
  size_t rest_size = block->size - sizeof(block_t) - size;
  block_t *new_block = (void *)block + sizeof(block_t) + rest_size;
  set_block(new_block, size, 0, block, block->next_phys, NULL, NULL);
  if (block->next_phys) {
    block->next_phys->prev_phys = new_block;
  }
  else if (*tail_block) {
    *tail_block = new_block;
  }
  block->next_phys = new_block;
  block->size = rest_size;
  return new_block;
}

// call sbrk for memory allocation
block_t *request_memory(size_t size, FunType fp, block_t **head_block,
                        block_t **tail_block) {
  if (size <= 0) return NULL;
  size_t alloc_size = (size + 2 * sizeof(block_t)) > ALLOC_UNIT
                          ? (size + sizeof(block_t))
                          : ALLOC_UNIT;
  void *ptr = (*fp)(alloc_size);
  if (ptr == (void *)-1) return NULL;

  block_t *block = ptr;
  set_block(block, size, 0, *tail_block, NULL, NULL, NULL);
  if (*tail_block) {
    (*tail_block)->next_phys = block;
  }
  if (alloc_size == ALLOC_UNIT) {
    block_t *free_block = ptr + sizeof(block_t) + size;
    set_block(free_block, alloc_size - (sizeof(block_t) * 2 + size), 0, block,
              NULL, NULL, NULL);
    block->next_phys = free_block;
    *tail_block = free_block;
    free_list_add_front(free_block, head_block);
  }
  else {
    block->next_phys = NULL;
    *tail_block = block;
  }
  return block;
}

// sbrk with lock
void *sbrk_lock(intptr_t size) {
  assert(pthread_mutex_lock(&lock) == 0);
  void *ptr = sbrk(size);
  assert(pthread_mutex_unlock(&lock) == 0);
  return ptr;
}

// policy to find block
block_t *find_bf(size_t size, block_t **head_block) {
  assert(head_block);
  block_t *optimal = NULL;
  block_t *curr = *head_block;
  while (curr) {
    if (curr->size > size + sizeof(block_t)) {
      // find suitable one, but may not the best one.
      if (optimal == NULL || curr->size < optimal->size) {
        optimal = curr;
      }
    }
    else if (curr->size >= size) {
      // A perfect block found
      return curr;
    }
    curr = curr->next_list;
  }
  return optimal;
}

// malloc space according to size and policy
void *_malloc(size_t size, FunType fp, block_t **head_block,
              block_t **tail_block) {
  if (*head_block == NULL) {
    block_t *block = request_memory(size, fp, head_block, tail_block);
    return block == NULL ? block : block + 1;
  }
  else {
    block_t *rslt = find_bf(size, head_block);
    if (rslt == NULL) {
      block_t *block = request_memory(size, fp, head_block, tail_block);
      return block == NULL ? block : block + 1;
    }
    else if (rslt->size <= size + sizeof(block_t)) { // perfect block
      free_list_remove(rslt, head_block);
      return rslt + 1;
    }
    else {
      block_t *new_block = split(rslt, size, tail_block);
      return new_block + 1;
    }
  }
}

// Merge block->next_phys into block, and form a greater one.
void phys_list_merge_next(block_t *block, block_t **tail_block) {
  assert(block && block->next_phys);
  assert((void *)block + sizeof(block_t) + block->size == block->next_phys);
  block_t *next_block = block->next_phys;
  assert(block->isFree && next_block->isFree);
  block->size += sizeof(block_t) + block->next_phys->size;
  if (next_block->next_phys) {
    next_block->next_phys->prev_phys = block;
  }
  else if (*tail_block) {
    *tail_block = block;
  }
  block->next_phys = next_block->next_phys;
}

// init block
void set_block(block_t *block, size_t size, int isFree, block_t *prev_phys,
               block_t *next_phys, block_t *prev_list, block_t *next_list) {
  block->size = size;
  block->isFree = isFree;
  block->prev_phys = prev_phys;
  block->next_phys = next_phys;
  block->prev_list = prev_list;
  block->next_list = next_list;
}
