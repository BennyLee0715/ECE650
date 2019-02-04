# Report 2: Thread-Safe Malloc

- Student Name: Yifan Men
- Student NetID: ym129

## 1. Overview of Implementation

### 1.1 General Idea on Adding Locks

No matter which version, we should initialize a lock before we try to lock and unlock. We can use the following code to create a lock:

```C
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
```

For LOCK_VERSION, the idea would be simple. We can take advantage of previous work, add lock immediately before and after `malloc` and `free`(see below), the section between `pthread_mutex_lock` and `pthread_mutex_unlock` are called critical section, this part would not run simultaneously. For other code run in threads allows concurrency. This LOCK_VERSION provides a `malloc` and `free` level concurrency.

```C
void *ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&lock);
  void *ptr = _malloc(size);
  pthread_mutex_unlock(&lock);
  return ptr;
}

void ts_free_lock(void *ptr) {
  pthread_mutex_lock(&lock);
  _free(ptr);
  pthread_mutex_unlock(&lock);
}
```

For NOLOCK_VERSION, we should only add lock right before and after `sbrk` call like the following `sbrk_lock` function, with no lock added at other places. The main idea of this version is to create a free list for each thread. On every thread, code are executed sequentially, so every free list is independent on each other, no overlapping memory region should appear. This NOLOCK_VERSION prodes a `sbrk` level concurrency.

```C
// sbrk of NOLOCK_VERSION
void *sbrk_lock(intptr_t size) {
  pthread_mutex_lock(&lock);
  void *ptr = sbrk(size);
  pthread_mutex_unlock(&lock);
  return ptr;
}
```

### 1.2 Improved Free List 

Based on work of HW01, I tried to add lock at appropriate place. For LOCK_VERSION, all four tests works fine, since every `malloc` and `free` call are separate. For NOLOCK_VERSION, it works for `thread_test`/`thread_test_malloc_free`/`thread_test_measurement` can pass every time, but it doesn't work for `thread_test_malloc_free_change_thread`, it can pass 43/50. I know there must be something wrong with this idea or even implementation.

For [b10c6f0 version](https://github.com/menyf/ECE650/tree/b10c6f0d304d9074c286e28c0e7f30cd2d68ec4a/HW02_thread-safe_malloc), assertion occurred at line 83, that "prev_phys->next_phys == block" failed, or sometime overlapping happens. I thought about this problem can result from two reasons: 
 
- For one, I changed the pointer from block to the physically adjacent one but did not connect back. I was thinking the place where such connection may occur. There were three places: 
  * `request_memory` function. It call `sbrk` for new space, I linked them together. 
  * `split` function. It simply narrow down the free block, assign part of the tail as `malloc`ed block. 
  * `phys_list_merge_next`. It will merge two adjacent physical bocks into one. More specifically, it will merge the block->next into a larger block. 
- For another possible error, other thread may change it. I checked all modification to pointer whose variable name end with '_phys'. I think no blocks in different threads can intersect. 
 
The last confusion is that, for `thread_test_malloc_free_change_thread.c` file, it malloc blocks and free some of them in threads , and then all threads joined into one. My policy would be, if we free a block, and it has exact physical adjacent block that created in the same thread, I will merge it into the adjacent block that can be freed in either main thread or before threads join together. In the end, the free list should consist of only blocks freed in main thread with no component of blocks freed before join. 

Pass rate:

**LOCK_VERSION**

 * thread_test 50/50
 * thread_test_malloc_free 50/50
 * thread_test_malloc_free_change_thread 50/50
 * thread_test_measurement 50/50
 
**NOLOCK_VERSION**

 * thread_test 50/50
 * thread_test_malloc_free 50/50
 * thread_test_malloc_free_change_thread **43/50**
 * thread_test_measurement 50/50

**This problem remains unsolved.**

### 1.3 Simple Free List

Can not figure out the reason why my previous solution doesn't work, I rewrite it with simple free list, and tested over 200 times with no failure.

My implementation would be naive. I used `head_block` and `tail_block` to trace the beginning free block and ending free block, all blocks are sorted in address-ascending order. Like what I mentioned previously, every thread has its own free list. When threads join together, it will form a new free list if new blocks are freed.

## 2. Result & Analysis

Based on 200 times experiments(all passed, results available at [lock.txt](https://github.com/menyf/ECE650/blob/master/HW02_thread-safe_malloc/thread_tests/lock.txt) and [unlock.txt](https://github.com/menyf/ECE650/blob/master/HW02_thread-safe_malloc/thread_tests/nolock.txt)), I conducted with the following results:
        
|  | LOCK | NO_LOCK |
| --- | --- | --- |
| Avg Time | 2.13s | 0.44s |
| Max Time | 2.87s | 0.56s |
| Min Time | 1.24s | 0.28s |
| Avg Size | 41609KB | 42161KB |
| Max Size | 44200KB | 43483KB |
| Min Size | 41493KB  | 41489KB |


Execution Time Analysis: Based on the result, NOLOCK version is four times faster than LOCK version. As mentioned previously, NOLOCK version would only lock during `sbrk` call, so for other operations like element adding or removal can happen simultaneously. Besides, for LOCK version, all threads share one singly free list, so the free list could be really long, making it hard to find the best fit one. For NO_LOCK version, each thread has its own free list, it will lead to an even more time-consuming situations, that is because the hit rate might be low, or an even more `sbrk` call would happen. However, according the experiment results, it is evident that the low hit rate may take the dominated factor.

Data Segment Size Analysis: Based on the results, there is not evident difference between two versions on size. That is, if a block can not hit on single free list(LOCK version), then it might not hit on multiple free lists(NOLOCK version) either. This proves my assumption of previous paragraph to be false. 

In conclusion, the operations of free list take dominated factor on time. There is no evident improvement of NOLOCK version on data segment size.

## 3. Other Lessons

### 3.1 Abstraction
For the two policies, I thought the difference between LOCK_VERSION and NOLOCK_VERSION. I think the only difference is where to put the lock and the definition of head and tail. Then I passed them as function pointer and pointer to pointer to avoid redundant code. I think this is a good use of abstraction. 

### 3.2 Assertion
When debugging, I felt very hard, so I added some self-evident assertions in my code, making sure the process is on right track in every assertion. This helps me debugging a lot.

### 3.3 Bash Shell

I find it hard to make everything and run the testing program one by one for many times, so I wrote an Bash shell script to help me:

```Bash
echo "Making"
make clean
make
for i in {1..200}
do
echo "Testing $i thread_test"
./thread_test
echo "Testing $i thread_test_malloc_free"
./thread_test_malloc_free
echo "Testing $i thread_test_malloc_free_change_thread"
./thread_test_malloc_free_change_thread 
echo "Testing $i thread_test_measurement"
./thread_test_measurement
done
echo "Cleaning"
make clean
```