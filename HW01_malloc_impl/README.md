# Malloc Implementation

## Background

Please see [requirement.pdf](https://github.com/menyf/ECE650/blob/master/HW01_malloc_impl/requirement.pdf) for specific requirement.

My report for this assignment is available at [report.md](https://github.com/menyf/ECE650/blob/master/HW01_malloc_impl/report.md). Please note my report was based on commit 8120e888bb1156a238871ea02031a4a7ac0532be. I added ALLOC_UNIT to further optimize this design, which is mentioned in [2.4 Further Improvements](https://github.com/menyf/ECE650/blob/master/HW01_malloc_impl/report.md#14-further-improvements). 

## Usage and Result

Tests were ran on Duke virtual machine with Ubuntu 18.

commit id: cad125082193953ffca183436df3de24f0713f43

```
make
cd alloc_policy_tests
make

# FF
$ ./small_range_rand_allocs
data_segment_size = 4079616, data_segment_free_space = 331824
Execution Time = 1.320798 seconds
Fragmentation  = 0.081337
$ ./equal_size_allocs
Execution Time = 1.661197 seconds
Fragmentation  = 0.456717
$ ./large_range_rand_allocs
Execution Time = 5.202420 seconds
Fragmentation  = 0.113902

# BF
$ ./small_range_rand_allocs
data_segment_size = 3919872, data_segment_free_space = 82352
Execution Time = 0.364347 seconds
Fragmentation  = 0.021009
$ ./equal_size_allocs
Execution Time = 1.656354 seconds
Fragmentation  = 0.456717
$ ./large_range_rand_allocs
Execution Time = 40.058784 seconds
Fragmentation  = 0.039797
```

## Tips

For this problem, we can situmulate the whole process. For a block of memory, it consists of two parts--meta information of the block, the block returned as the space to store data. 

Here I am using free list to implement. I also tried with ALLOCATION_UNIT but failed. When I was doing this homework, a TA mentioned several things worth noticinng: 

- Did you correctly search the free list? (If the search always fail, you always have to go through the entire list and call sbrk for more memory.)
- Did you call sbrk only occasionally? (You should call sbrk to get a rather large amount of memory and split the free space.)
- Did you perform the coalescing correctly? (Otherwise the freelist will build up and takes longer to search.)
