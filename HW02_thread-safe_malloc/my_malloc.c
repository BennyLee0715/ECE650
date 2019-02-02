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
  return _malloc(size, sbrk_lock, &head_block_tls, &tail_block_tls);
}

void ts_free_nolock(void *ptr) {
  _free(ptr, &head_block_tls, &tail_block_tls);
}

void _free(void *ptr, block_t **head_block, block_t **tail_block) {
  if (ptr == NULL) return;
  block_t *block = (block_t *)ptr - 1;
  free_list_insert(block, head_block, tail_block);
  free_list_merge(block, head_block, tail_block);
}

// Add a block to the front of the list
void free_list_insert(block_t *block, block_t **head_block,
                      block_t **tail_block) {
  assert(block && block->prev_list == NULL && block->next_list == NULL);
  assert(block != *head_block);
  if (*head_block == NULL) { // free list is blank
    *head_block = block;
    *tail_block = block;
  }
  else if ((unsigned long)block < (unsigned long)*head_block) {
    // found a front one.
    block->next_list = *head_block;
    (*head_block)->prev_list = block;
    *head_block = block;
  }
  else {
    block_t *curr = *head_block;
    while (curr->next_list) {
      assert((unsigned long)curr + sizeof(block_t) + curr->size <
             (unsigned long)curr->next_list);
      if ((unsigned long)curr < (unsigned long)block &&
          (unsigned long)block < (unsigned long)curr->next_list)
        break;
      curr = curr->next_list;
    }
    block->next_list = curr->next_list;
    block->prev_list = curr;
    if (curr->next_list)
      curr->next_list->prev_list = block;
    else
      *tail_block = block;
    curr->next_list = block;
  }
}

// Remove a block from list
void free_list_remove(block_t *block, block_t **head_block,
                      block_t **tail_block) {
  assert(block);
  if (block->prev_list) {
    block->prev_list->next_list = block->next_list;
  }
  else {
    *head_block = block->next_list;
  }
  if (block->next_list) {
    block->next_list->prev_list = block->prev_list;
  }
  else {
    *tail_block = block->prev_list;
  }
  block->next_list = NULL;
  block->prev_list = NULL;
}

// Megre adjacent free blocks around 'block'
void free_list_merge(block_t *block, block_t **head_block,
                     block_t **tail_block) {
  block_t *prev = block->prev_list;
  block_t *next = block->next_list;
  assert(next == NULL || (unsigned long)block + sizeof(block_t) + block->size <=
                             (unsigned long)next);
  assert(prev == NULL || (unsigned long)prev + sizeof(block_t) + prev->size <=
                             (unsigned long)block);

  if (next && (void *)block + sizeof(block_t) + block->size == next) {
    block->size += sizeof(block_t) + next->size;
    block->next_list = next->next_list;
    if (next->next_list) {
      next->next_list->prev_list = block;
    }
    else {
      *tail_block = block;
    }
  }

  if (prev && (void *)prev + sizeof(block_t) + prev->size == block) {
    prev->size += sizeof(block_t) + block->size;
    prev->next_list = block->next_list;
    if (block->next_list) {
      block->next_list->prev_list = prev;
    }
    else {
      *tail_block = prev;
    }
  }
}

// Split block into two, the meta of block still accounts for
// free block, the rest part is regarded as malloced one.
block_t *split(block_t *block, size_t size) {
  assert(block && block->size >= sizeof(block_t) + size);
  size_t rest_size = block->size - sizeof(block_t) - size;
  assert(rest_size > 0);
  block_t *new_block = (void *)block + sizeof(block_t) + rest_size;
  set_block(new_block, size, NULL, NULL);
  block->size = rest_size;
  assert((void *)block + block->size + sizeof(block_t) == new_block);
  return new_block;
}

// call sbrk for memory allocation
block_t *request_memory(size_t size, FunType fp, block_t **head_block,
                        block_t **tail_block) {
  if (size <= 0) return NULL;
  size_t alloc_size = size + sizeof(block_t);
  void *ptr = (*fp)(alloc_size); // sbrk call
  if (ptr == (void *)-1) return NULL;
  block_t *block = ptr;
  set_block(block, size, NULL, NULL);
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
  assert(*head_block);
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
      free_list_remove(rslt, head_block, tail_block);
      return rslt + 1;
    }
    else {
      block_t *new_block = split(rslt, size);
      return new_block + 1;
    }
  }
}

// init block
void set_block(block_t *block, size_t size, block_t *prev_list,
               block_t *next_list) {
  block->size = size;
  block->prev_list = prev_list;
  block->next_list = next_list;
}