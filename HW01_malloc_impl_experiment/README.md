# Malloc Implementation

## Background

Please see [requirement.pdf](https://github.com/menyf/ECE650/blob/master/HW01_malloc_impl/requirement.pdf) for specific requirement.

With great improvements mentions in [report.md](https://github.com/menyf/ECE650/blob/master/HW01_malloc_impl/report.md). This version is much more faster.

## Usage and Result

Tests were ran on Duke virtual machine with Ubuntu 18.

```
# BF
$ ./small_range_rand_allocs
data_segment_size = 3915520, data_segment_free_space = 80016
Execution Time = 0.512221 seconds
Fragmentation  = 0.020436
$ ./equal_size_allocs
Execution Time = 3.513114 seconds
Fragmentation  = 0.450000
$ ./large_range_rand_allocs
Execution Time = 97.019144 seconds
Fragmentation  = 0.039970

# FF
$ ./small_range_rand_allocs
data_segment_size = 4076160, data_segment_free_space = 329920
Execution Time = 1.870334 seconds
Fragmentation  = 0.080939
$ ./equal_size_allocs
Execution Time = 3.439875 seconds
Fragmentation  = 0.450000
$ ./large_range_rand_allocs
Execution Time = 12.230138 seconds
Fragmentation  = 0.114686
```

## Tips

For this problem, we can situmulate the whole process. For a block of memory, it consists of two parts--meta information of the block, the block returned as the space to store data. 

Here I am using free list to implement. I also tried with ALLOCATION_UNIT but failed. When I was doing this homework, a TA mentioned several things worth noticinng: 

- Did you correctly search the free list? (If the search always fail, you always have to go through the entire list and call sbrk for more memory.)
- Did you call sbrk only occasionally? (You should call sbrk to get a rather large amount of memory and split the free space.)
- Did you perform the coalescing correctly? (Otherwise the freelist will build up and takes longer to search.)