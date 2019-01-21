#include "my_malloc.h"

block_t *request_memory(size_t size) {
  void *ptr = sbrk(size + sizeof(block_t));
  if (ptr == (void *)-1) {
    return NULL;
  }
  block_t *block = ptr;
  block->next = NULL;
  block->size = size;
  block->isFree = 0;
  return block;
}

// First Fit malloc/free
void *ff_malloc(size_t size) {
  // invalid parameter check
  if (size <= 0)
    return NULL;

  block_t *block;
  if (head_block == NULL) { // no previous blocks
    block = request_memory(size);
    if (block == NULL)
      return NULL;
    head_block = block;
  } else {
    block_t *curr = head_block;
    block_t *last = NULL;

    // find the first fit
      while(curr
            && (curr->isFree == 0 // unavailable
                || curr->size < size) // not enough space, note that it should be devided into two parts
      {
      last = curr;
      curr = curr->next;
      }

      if(curr == NULL) { // append to the end of data segment
      block = request_memory(size);
      if (block == NULL)
        return NULL;
      last->next = block;
      }
      else { // insert into some freed space
      if (curr->size < size + sizeof(block_t)) { // can not be devided into two blocks
        curr->isFree = 0;
      } else {
        block_t *next_block = (void *)curr + size + sizeof(block_t);
        next_block->isFree = 1;
        next_block->size = curr->next - next_block - sizeof(block_t);
        next_block->next = curr->next;

        curr->next = next_block;
        curr->isFree = 0;
        curr->size = size;
      }
      block = curr;
      }
  }
  return block + 1; // skip meta info
}

void ff_free(void *ptr) { _free(ptr); }

// Best Fit malloc/free
void *bf_malloc(size_t size) {
  // invalid parameter check
  if (size <= 0)
    return NULL;

  block_t *block;
  if (head_block == NULL) { // no previous blocks
    block = request_memory(size);
    if (block == NULL)
      return NULL;
    head_block = block;
  } else {
    block_t *curr = head_block;
    block_t *optimal_curr = NULL;
    block_t *optimal_last = NULL;
    block_t *last = NULL;

    // find the best fit
    while (curr) {
      if (curr->isFree == 1 && curr->size >= size) {
        if (optimal_curr == NULL || curr->size < optimal_curr->size) {
          optimal_curr = curr;
          optimal_last = last;
        }
      }
      last = curr;
      curr = curr->next;
    }

    if (optimal_curr == NULL) { // append to the end of data segment
      block = request_memory(size);
      if (block == NULL)
        return NULL;
      last->next = block;
    } else { // insert into some freed space
      if (optimal_curr->size > size + sizeof(block_t)) {
        block_t *next_block = (void *)optimal_curr + size + sizeof(block_t);
        next_block->isFree = 1;
        next_block->size = optimal_curr->next - next_block - sizeof(block_t);
        next_block->next = optimal_curr->next;

        curr->next = next_block;
        curr->isFree = 0;
        curr->size = size;
      } else {
        curr->isFree = 0;
      }
      block = curr;
    }
  }
  return block + 1; // skip meta info
}

void bf_free(void *ptr) { _free(ptr); }

void _free(void *ptr) {
    if(ptr == NULL) return;
    block_t* block = (block_t *)ptr - 1;
    block->isFree = 1;
    if(block->next && block->next->isFree == 1) { // merge forward
        block->size += sizeof(block_t) + block->next->size;
        block->next = block->next->next;
        if(block->next){
            block->next->prev = block;
        }
    }
    if(block->prev && block->prev->isFree == 1) { // merge backward
        block->prev->size += sizeof(block_t) + block->size;
        block->prev->next = block->next;
        if(block->next){
            block->next->prev = block->prev;
        }
    }
}