#include "my_malloc.h"
#include "assert.h"

block_t *request_memory(size_t size) {
  size_t alloc_size = size + sizeof(block_t);
  void *ptr = sbrk(alloc_size);
  if (ptr == (void *)-1) {
    return NULL;
  }
  data_segment_size += alloc_size;
  block_t *block = ptr;
  block->next = NULL;
  block->prev = NULL;
  block->size = alloc_size - sizeof(block_t);
  return block;
}

block_t *find_ff(size_t size) {
  block_t *curr = head_block;
  while (curr && curr->size < size) {
    curr = curr->next;
  }
  return curr;
}

block_t *find_bf(size_t size) {
  block_t *curr = head_block;
  block_t *optimal_curr = NULL;
  while (curr) {
    if (curr->size > size + sizeof(block_t)) {
      if (optimal_curr == NULL || curr->size < optimal_curr->size) {
        optimal_curr = curr;
      }
    } else if (curr->size >= size) { // perfect block found
      return curr;
    }
    curr = curr->next;
  }
  return optimal_curr;
}

void *_malloc(size_t size, FunType fp) {
  if (head_block == NULL) { // no available freed blocks
    block_t *block = request_memory(size);
    return block == NULL ? NULL : block + 1;
  } else {
    // search in free list
    // if found a fit, apply to it(either split or take up it)
    // else allocate one
    block_t *curr = (*fp)(size);

    if (curr == NULL) {
      // no fit found
      block_t *block = request_memory(size);
      return block == NULL ? block : block + 1;
    } else if (curr->size < size + sizeof(block_t)) {
      // can not be split into two
      // give the rest to user for free
      free_list_remove(curr);
      return curr + 1;
    } else {
      // split into two
      // note: a block with 0-size is acceptable here
      block_t *malloced_block =
          split(curr, curr->size - size - sizeof(block_t));
      return malloced_block + 1;
      // free_list_remove(curr);
      // free_list_add(rest);
      // return curr + 1;
    }
  }
}

// First Fit malloc/free
void *ff_malloc(size_t size) { _malloc(size, find_ff); }
void ff_free(void *ptr) { _free(ptr); }

// Best Fit malloc/free
void *bf_malloc(size_t size) { _malloc(size, find_bf); }
void bf_free(void *ptr) { _free(ptr); }

// Performance study
unsigned long get_data_segment_size() { return data_segment_size; }
unsigned long get_data_segment_free_space_size() {
  return data_segment_free_space_size;
}

// Customized functions
void _free(void *ptr) {
  if (ptr == NULL)
    return;
  block_t *block = (block_t *)ptr - 1;
  free_list_add(block);
  free_list_merge(block);
}

void free_list_add(block_t *block) {
  data_segment_free_space_size += block->size + sizeof(block_t);

  block->next = NULL;
  block->prev = NULL;
  if (head_block == NULL) { // no freed blocks
    head_block = block;
  } else if ((unsigned long)head_block > (unsigned long)block) { // add to front
    block->next = head_block;
    head_block->prev = block;
    head_block = block;
  } else { // add in some place other than the front
    block_t *curr = head_block;
    while (curr->next && (unsigned long)curr->next < (unsigned long)block) {
      curr = curr->next;
    }
    if (curr->next != NULL) {
      curr->next->prev = block;
    }
    block->prev = curr;
    block->next = curr->next;
    curr->next = block;
  }
}

void free_list_remove(block_t *block) {
  assert((unsigned long)block >= (unsigned long)head_block);
  data_segment_free_space_size -= block->size + sizeof(block_t);
  if (block->prev == NULL && block->next == NULL) {
    head_block = NULL;
  } else if (block->prev == NULL) {
    head_block = block->next;
    block->next->prev = NULL;
  } else if (block->next == NULL) {
    block->prev->next = NULL;
  } else {
    block->prev->next = block->next;
    block->next->prev = block->prev;
  }
  block->prev = NULL;
  block->next = NULL;
}

void free_list_merge(block_t *curr) {
  assert(curr != NULL);
  block_t *prev = curr->prev;
  if (prev && (void *)prev + sizeof(block_t) + prev->size == (void *)curr) {
    prev->size += sizeof(block_t) + curr->size;
    prev->next = curr->next;
    if (curr->next) {
      curr->next->prev = prev;
    }
    curr = prev;
  }

  block_t *next = curr->next;
  if (next && (void *)next == (void *)curr + sizeof(block_t) + curr->size) {
    curr->size += sizeof(block_t) + next->size;
    curr->next = next->next;
    if (next->next) {
      next->next->prev = curr;
    }
  }
}

block_t *split(block_t *curr, size_t sz) {
  data_segment_free_space_size -= curr->size - sz;
  size_t tot = curr->size;
  curr->size = sz;
  block_t *malloced_block = (void *)curr + curr->size + sizeof(block_t);
  malloced_block->size = tot - sz - sizeof(block_t);
  malloced_block->prev = NULL;
  malloced_block->next = NULL;
  return malloced_block;
}
