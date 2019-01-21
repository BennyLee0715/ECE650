#include "my_malloc.h"
#include "assert.h"

block_t *request_memory(size_t size) {
  void *ptr = sbrk(size + sizeof(block_t));
  if (ptr == (void *)-1) {
    return NULL;
  }
  data_segment_size += size + sizeof(block_t);
  block_t *block = ptr;
  block->next = NULL;
  block->prev = NULL;
  block->size = size;
  //printf("[Info] Block space: %10p - %10p", block, ptr + size + sizeof(block_t));
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
    if (curr->size >= size) {
      if (optimal_curr == NULL || curr->size < optimal_curr->size) {
        optimal_curr = curr;
      }
    }
    curr = curr->next;
  }
  return optimal_curr;
}

void *_malloc(size_t size, FunType fp) {
    //printf("When mallocing %ld\n", size);
    stat("");
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
      block_t *rest = split(curr, size);
      //free_list_remove(curr);
      //free_list_add(rest);
      return curr + 1;
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
  //printf("Before freeing\n");
  stat("");
  block_t *block = (block_t *)ptr - 1;
  //printf("I am freeing %ld ints %p - %p\n", block->size / 4, block, (void *)block + sizeof(block_t) + block->size);
  free_list_add(block);
  //printf("In the middle of freeing\n");
  stat("");
  free_list_merge();
  //printf("After freeing\n");
  stat("");
}

void free_list_add(block_t *block) {
  data_segment_free_space_size += block->size + sizeof(block_t);

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
    if (curr->next) {
      curr->next->prev = block;
    }
    block->prev = curr;
    block->next = curr->next;
    curr->next = block;

  }
}

void free_list_remove(block_t *block) {
  assert((unsigned long)block >= (unsigned long)head_block);
  data_segment_free_space_size -= head_block->size + sizeof(block_t);

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

void free_list_merge() {
  block_t *curr = head_block;
  assert(curr != NULL);
  while (curr && curr->next) {
    block_t *temp = curr->next;
    unsigned long addr_next = (unsigned long)curr + sizeof(block_t) + curr->size ;
    unsigned long addr_temp = (unsigned long)temp;
    if (addr_next == addr_temp) {
      curr->size += sizeof(block_t) + temp->size;
      curr->next = temp->next;
      if (temp->next != NULL)
        temp->next->prev = curr;

      temp = curr->next;
      addr_next = (unsigned long)curr + sizeof(block_t) + curr->size ;
      addr_temp = (unsigned long)temp;
      if (temp && addr_next == addr_temp) {
          curr->size += sizeof(block_t) + temp->size;
          curr->next = temp->next;
          if (temp->next != NULL){
              assert(temp->next != NULL);
              temp->next->prev = curr;
          }
      }
    }
    curr = curr->next;
  }
}

block_t *split(block_t *curr, size_t sz) {
  block_t *rest = (void *)curr + sizeof(block_t) + sz;

  rest->prev = curr->prev;
  if (curr->prev) { // not head
    curr->prev->next = rest;
  } else { // head
    head_block = rest;
  }

  rest->next = curr->next;
  if (curr->next) { // not tail
    curr->next->prev = rest;
  }

  curr->next = NULL;
  curr->prev = NULL;
  return rest;
}

void stat(const char *str){
    //printf("%s\n", str);
    block_t * curr = head_block;
    int cnt = 0;
    while(curr) {
        //printf("Block %d is at %10p - %10p, with size of %ld\n", ++cnt, curr, (void *)curr + curr->size + sizeof(block_t), curr->size);
        curr = curr->next;
    }
    //printf("============================================================\n");
}