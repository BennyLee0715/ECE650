# Malloc Implementation

## Background

Please see [requirement.pdf](https://github.com/menyf/ECE650/blob/master/HW01_malloc_impl/requirement.pdf) for specific requirement.

My report for this assignment is available at [report.md](https://github.com/menyf/ECE650/blob/master/HW01_malloc_impl/report.md)

## Usage and Result

Tests were ran on Duke virtual machine with Ubuntu 18.

```
make
cd alloc_policy_tests
make

# FF
vcm@vcm-8126:~/ece650/HW01_malloc_impl/alloc_policy_tests$ ./small_range_rand_allocs
data_segment_size = 4076160, data_segment_free_space = 329920
Execution Time = 1.745480 seconds
Fragmentation  = 0.080939
vcm@vcm-8126:~/ece650/HW01_malloc_impl/alloc_policy_tests$ ./equal_size_allocs
Execution Time = 1.781792 seconds
Fragmentation  = 0.450000
vcm@vcm-8126:~/ece650/HW01_malloc_impl/alloc_policy_tests$ ./large_range_rand_allocs
Execution Time = 12.323683 seconds
Fragmentation  = 0.114686

# BF
vcm@vcm-8126:~/ece650/HW01_malloc_impl/alloc_policy_tests$ ./small_range_rand_allocs
data_segment_size = 3915520, data_segment_free_space = 80016
Execution Time = 0.366520 seconds
Fragmentation  = 0.020436
vcm@vcm-8126:~/ece650/HW01_malloc_impl/alloc_policy_tests$ ./equal_size_allocs
Execution Time = 1.796241 seconds
Fragmentation  = 0.450000
vcm@vcm-8126:~/ece650/HW01_malloc_impl/alloc_policy_tests$ ./large_range_rand_allocs
Execution Time = 93.784540 seconds
Fragmentation  = 0.039970
```

## Tips

For this problem, we can situmulate the whole process. For a block of memory, it consists of two parts--meta information of the block, the block returned as the space to store data. 

Here I am using free list to implement. I also tried with ALLOCATION_UNIT but failed. When I was doing this homework, a TA mentioned several things worth noticinng: 

- Did you correctly search the free list? (If the search always fail, you always have to go through the entire list and call sbrk for more memory.)
- Did you call sbrk only occasionally? (You should call sbrk to get a rather large amount of memory and split the free space.)
- Did you perform the coalescing correctly? (Otherwise the freelist will build up and takes longer to search.)
