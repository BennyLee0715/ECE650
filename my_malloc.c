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
  return block;
}

block_t * find_ff(){
    block_t * curr = head_block;
    while(curr && curr->size < size){
        curr = curr->next;
    }
    return curr;
}

block_t * find_bf(){
  block_t * curr = head_block;
  block_t * optimal_curr = NULL;
  while(curr){
      if(curr->size >= size){
          if(optimal_curr == NULL || curr->size < optimal_curr->size){
              optimal_curr = curr;
          }
      }
      curr = curr->next;
  }
  return optimal_curr;
}

void * _malloc(size_t size, FunType fp){
    if(head_block == NULL){ // no available freed blocks
        block_t * block = request_memory(size);
        return block == NULL ? block : block + 1;
    }
    else{
        // search in free list
        // if found a fit, apply to it(either split or take up it)
        // else allocate one
        block_t *curr = fp();

        if(curr == NULL) {
            // no fit found
            block_t * block = request_memory(size);
            return block == NULL ? block : block + 1;
        }
        else if(curr->size < size + sizeof(block_t)){
            // can not be split into two
            // give the rest to user for free
            free_list_remove(curr);
            return curr + 1;
        }
        else {
            // split into two
            // note: a block with 0-size is acceptable here
            free_list_remove(curr);
            block_t *rest = split(curr, size);
            free_list_add(rest);
            return curr + 1;
        }
    }
}

//First Fit malloc/free
void *ff_malloc(size_t size){
    _malloc(size, find_ff);

}
void ff_free(void *ptr){
    _free(ptr);
}

//Best Fit malloc/free
void *bf_malloc(size_t size){
    _malloc(size, find_bf);
}
void bf_free(void *ptr){
    _free(ptr);
}

//Performance study
unsigned long get_data_segment_size(){
    return data_segment_size;
}
unsigned long get_data_segment_free_space_size() {
    return data_segment_free_space_size;
}

// Customized functions
void _free(void *ptr) {
    if(ptr == NULL) return;
    block_t *block = ptr;
    free_list_add(ptr);
    free_list_merge();
}

void free_list_add(block_t *block) {
    data_segment_free_space_size += head_block->size + sizeof(block_t);

    if (head_block == NULL) { // no freed blocks
        head_block = block;
    }
    else if((unsigned long)head_block > (unsigned long) block){ // add to front
        block->next = head_block;
        head_block->prev = block;
        head_block = block;
    }
    else { // add in some place other than the front
        block_t *curr = head_block;
        while(curr->next && (unsigned long) curr->next < (unsigned long) block){
            curr = curr->next;
        }
        if(curr->next) {
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

    if(block->prev == NULL && block->next == NULL) {
        head_block = NULL;
    }
    else if(block->prev == NULL) {
        head_block = block->next;
        block->next->prev = NULL;
    }
    else if(block->next == NULL) {
        block->prev->next = NULL;
    }
    else {
        block->prev->next = block->next;
        block->next->prev = block->prev;
    }
    block->prev = NULL;
    block->next = NULL;
}

void free_list_merge(){
    block_t *curr = head_block;
    while(curr->next){
        block_t *temp = curr->next;
        if((unsigned long)curr + sizeof(block_t) + curr->size == (unsigned long) temp){
            curr->size += sizeof(block_t) + temp->size;
            curr->next = temp->next;
            if(temp->next) temp->next->prev = curr;
        }
        curr = curr->next;
    }
}

block_t *split(block_t *curr, size_t sz) {
    block_t * rest = (void *)curr + sizeof(block_t) + sz;

    rest->prev = curr->prev;
    if(curr->prev){ // not head
        curr->prev->next = rest;
    }
    else { // head
        head_block = rest;
    }

    rest->next = curr->next;
    if(curr->next){ // not tail
        curr->next->prev = rest;
    }

    curr->next = NULL;
    curr->prev = NULL;
    return rest;
}