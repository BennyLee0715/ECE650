# Malloc Implementation

## Background

This is Homework 1 for Duke ECE650 2019 spring.

## Usage and Result

Tests were ran on Duke virtual machine with Ubuntu18

```
make
cd alloc_policy_tests
make
# FF
~ $ ./small_range_rand_allocs
Execution Time = 17.032398 seconds
Fragmentation  = 0.075614
~ $ ./equal_size_allocs
Execution Time = 28.754213 seconds
Fragmentation  = 0.450000
~ $ ./large_range_rand_allocs
Execution Time = 109.246684 seconds
Fragmentation  = 0.094561
# BF
~ $ ./small_range_rand_allocs
Execution Time = 5.557294 seconds
Fragmentation  = 0.027893
~ $ ./equal_size_allocs
Execution Time = 18.465461 seconds
Fragmentation  = 0.450000
~ $ ./large_range_rand_allocs
Execution Time = 137.826278 seconds
Fragmentation  = 0.040785
```

## Tips

For this problem, we can situmulate the whole process. For a block of memory, it consists of two parts--meta information of the block, the block returned as the space to store data. 

Here I am using free list to implement. I also tried with ALLOCATION_UNIT but failed. When I was doing this homework, a TA mentioned several things worth noticinng: 

- Did you correctly search the free list? (If the search always fail, you always have to go through the entire list and call sbrk for more memory.)
- Did you call sbrk only occasionally? (You should call sbrk to get a rather large amount of memory and split the free space.)
- Did you perform the coalescing correctly? (Otherwise the freelist will build up and takes longer to search.)