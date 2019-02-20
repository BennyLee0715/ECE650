# TCP Socket Programming

## Background

Please see [homework3-program.pdf](https://github.com/menyf/ECE650/blob/master/HW03_tcp_socket/homework3-program.pdf) for specific requirement. 


## Usage and Result

Tests were ran on Duke virtual machine with Ubuntu 18.

commit id: 5fce049 

### Compilation

```
cd HW03_tcp_socket/src
make
```

### Ringmaster

Ringmaster program can be invoked with the format `./ringmaster <port_num> <num_players> <num_hops>`

For example, the following command will create a master listening on port 1234, allowing 50 players connected and 50 hops for potato.

```
./ringmaster 1234 50 500
```

### Player

Player program can be invoked with the format `./player <machine_name> <port_num>` 

For example, the following command will create a player connecting to master `vcm-1234.vm.duke.edu` at port `1234`

```
./player vcm-1234.vm.duke.edu 1234
```

You can also create multiple player in one terminal with the command `./client.sh <machine_name> <port_num> <player_num>` 

For example, the following command will create 25 player connecting to master `vcm-1234.vm.duke.edu` at port `1234`

```
./client.sh vcm-1234.vm.duke.edu 1234 25
```

Please note that the players should be able to run on different machines.

## Tips on Implementation

### Port Assignment
There are three ways to assigning a listening port to a server.

First, you can assign a specific port as a listening port. Usually, there are a wide range of ports which are unoccupied. The short of this way is you may assign a port that is already in used, but it is not frequent.

Second, you can assign a port from some start number, keep binding it, if it is already in use, then you can not bind it successfully. Keep trying it, you can always find one that is available.

Third and is the recommended one, you can specify `0` in `sin_port` when calling `bind`, the OS will assign a port for you.  After that, you can use `getsockname` to see which port the socket is bound to.

### Get IP

There are two ways to get your ip.

First, you can use `gethostname()` and `gethostbyname()` to get your hostname, and send it as message.

Second, you can find the client's ip address because `accept` syscall will fill in the `addr` argument, which is a `sockaddr` struct containing the client's IP address.

### Why Serialization is Meaningless

Essentially, data transmitted among different machines is byte stream. It may affect the result if you transmit a piece of data from a 32-bit machine to 64-bit machine, because they have different way of interpretation. However, under this homework, all machines are visualized in the same way by Duke Virtual Machine Manager. Besides, no matter which type the data you transmit, either `char *` or `struct`, it is identical byte data. Even though you serialize the data to a string, then it will result in less space, but it is still large enough to some problem. When I was testing my program, I found that if I send a piece of data with 2500 bytes, it would be divided into two piece of data, whose sizes are 1448bytes and 1052 bytes. This is the key to this problem. Serialization can not always decrease the size of data to an acceptable scale, that is 1448 bytes.

The solutions to this problem is relatively easy. For one, we can ask `recv` function to receive in a `while` loop, when the total length of data reaches expectation, we got the whole piece of data. Or, we can set a flag in `recv` function like `recv(fd, &potato, sizeof(potato), MSG_WAITALL);` to ask program block until it meets such amount of data.

### Why Program should sleep before termination

At last, the master needs to tell each player to end the game. However, when an endpoint of socket close, it will send a `0` to the other endpoint. More specifically, a player may receive the `0` from other player before it receive the termination signal from master. Therefore, we need to make everyone get to "ready to terminate" status before they close the file descriptor.

## Reference

- Man Page
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/multi/index.html)
- [TCP fragmentation? (@ 1448 bytes)](https://forum.unity.com/threads/tcp-fragmentation-1448-bytes.235815/)
