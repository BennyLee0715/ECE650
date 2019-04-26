# Kernel Programming

grade: 100/100

## Background

Please see [requirement.pdf](https://github.com/menyf/ECE650/blob/master/HW05_Kernel-prog/requirement.pdf) for specific requirement. 


## Usage

Tests were ran on Duke virtual machine with **Ubuntu 16**.

commit id: a0996b8


```
# build
cd HW05_Kernel-prog/code/
make # build sneaky_mod
make sneaky_process # build sneaky_process

# Run
sudo ./sneaky_process
```

The `sneaky_process` quits with `q`. 

You should have two terminals open at the same time. For one is running sneaky_process, it accounts for insertion of `sneaky_mod`. For another is a normal terminal, you should be able to run the command in `requirement.pdf` with the sneaky results.

To simply insert the mod to kernel without running `sneaky_process`, you should use `sudo insmod sneaky_mod.ko pid="1111"` where `1111` is the pid you want to hide. To remove the mod, use `sudo rmmod sneaky_mod`.

## Tips

### Output

For the output of `printk`, it should be shown in `/var/log/syslog`.

You can use this command to show the last 30 lines: `sudo tail -n 30 /var/log/syslog`


### Pointer

The pointers may vary according to differnt systems, they should be changed accordingly in [sneaky_mod.c](https://github.com/menyf/ECE650/blob/master/HW05_Kernel-prog/code/sneaky_mod.c). Try to find the corresponding function pointers for [pages_rw](https://github.com/menyf/ECE650/blob/a0996b892ccc788b9aaf0fbb85701cf41dcf38ab/HW05_Kernel-prog/code/sneaky_mod.c#L35) and [pages_ro](https://github.com/menyf/ECE650/blob/a0996b892ccc788b9aaf0fbb85701cf41dcf38ab/HW05_Kernel-prog/code/sneaky_mod.c#L36), and also the pointer for [sys_call_table](https://github.com/menyf/ECE650/blob/a0996b892ccc788b9aaf0fbb85701cf41dcf38ab/HW05_Kernel-prog/code/sneaky_mod.c#L41) to make sure it matches with your Ubuntu version. 

The way to find them are (the method is also mentioned in the comment in  [sneaky_mod.c](https://github.com/menyf/ECE650/blob/master/HW05_Kernel-prog/code/sneaky_mod.c) file):

```
sudo cat /boot/System.map-4.4.0-142-generic | grep -e set_pages_ro -e set_pages_rw -e sys_call_table
ffffffff81071fc0 T set_pages_ro
ffffffff81072040 T set_pages_rw
ffffffff81a00200 R sys_call_table
ffffffff81a01560 R ia32_sys_call_table
```

### Header File and strnstr

For kernel programming, we are using `linux/string.h` instead of `string.h`. When we have a buffer with some data, it is simply the data, we don't know whether there is a null terminator `\0` inside. If we simply use `strstr`, it keeps searching until a null terminator, but such a null terminator could be part of the useful data in buffer. Therefore, a `strnstr` is needed to reslove this problem elegantly.

### Memory Function

`memcpy` does not support memory overlap, so use `memmove` instead of `memcpy`.


## Reference

- [copy_to_user](https://www.fsl.cs.sunysb.edu/kernel-api/re256.html)
- [Passing Command Line Arguments to a Module](http://www.tldp.org/LDP/lkmpg/2.6/html/x323.html)