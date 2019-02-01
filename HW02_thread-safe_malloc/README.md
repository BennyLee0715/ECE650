# Malloc Implementation

## Background

Please see [requirement.pdf](https://github.com/menyf/ECE650/blob/master/HW02_thread-safe_malloc/requirement.pdf) for specific requirement.

My report for this assignment is available at [report.md](https://github.com/menyf/ECE650/blob/master/HW02_thread-safe_malloc/report.md). Please note my report was based on commit 7a30eeaf9225ce16d78f17a0dc525014dfdaf128. 

For this assignment, I tried to take the same policy as [HW01\_malloc\_impl](https://github.com/menyf/ECE650/tree/master/HW01_malloc_impl) but failed to do so, there will always be some tests failed on thread\_test\_malloc\_free\_change\_thread, pass rate for this test is about 45/50. For the other test, my code can pass. The last version of HW01 policy would be [commit b10c6f0d304d9074c286e28c0e7f30cd2d68ec4a](https://github.com/menyf/ECE650/tree/b10c6f0d304d9074c286e28c0e7f30cd2d68ec4a). 

So I changed to only doubly free list method from a81ec410c1becec1146f84832198ff7b736ae3ac.

## Usage and Result

Tests were ran on Duke virtual machine with Ubuntu 18.

commit id: 7a30eeaf9225ce16d78f17a0dc525014dfdaf128

```
cd HW02_thread-safe_malloc
make
./run.sh
```

By default, it will test LOCK\_VERSION, you can change MALLOC\_VERSION to NOLOCK\_VERSION in Makefile of thread_tests directory.

## Tips

- [Thread-local Storage](https://en.wikipedia.org/wiki/Thread-local_storage) is helpful. It enable you to store independent free lists for different threads.
- You should also be familiar with synchronization primitives like pthread\_mutex\_t.
