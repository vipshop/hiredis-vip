# Hiredis-cluster

Hiredis-cluster is a C client library for cluster deployments of the [Redis](http://redis.io/) database.

Hiredis-cluster is using [Hiredis](https://github.com/redis/hiredis) for the connections to each Redis node.

Hiredis-cluster is a fork of Hiredis-vip, with the following improvements:

* The C library `hiredis` is an external dependency rather than a builtin part of the cluster client, meaning that the latest `hiredis` can be used.
* Support for SSL/TLS introduced in Redis 6
* Support for IPv6
* Using CMake as build system
* Code style guide (using clang-format)
* Improved testing
* Memory leak corrections

## Features

* Redis Cluster
    * Connect to a Redis cluster and run commands.

* Multi-key commands
    * Support `MSET`, `MGET` and `DEL`.
    * Multi-key commands will be processed and sent to slot owning nodes.

* Pipelining
    * Send multiple commands at once to speed up queries.
    * Supports multi-key commands described in above bullet.

* Asynchronous API
    * Send commands asynchronously and let a callback handle the response.
    * Needs an external event loop system that can be attached using an adapter.

* SSL/TLS
    * Connect to Redis nodes using SSL/TLS (supported from Redis 6)

* IPv6
    * Handles clusters on IPv6 networks

## Build instructions

Prerequisites:

* A C compiler (GCC or Clang)
* CMake and GNU Make
* (hiredis)[https://github.com/redis/hiredis]; downloaded automatically by
  default, but see build options below
* (libevent)[https://libevent.org/] (`libevent-dev` in Debian); can be avoided
  if building without tests (DISABLE_TESTS=ON)
* OpenSSL (`libssl-dev` in Debian) if building with TLS support

Building hiredis-cluster and its test suites requires headerfiles and linkage to (hiredis)[https://github.com/redis/hiredis]
and .

Hiredis-cluster will be built as a shared library and the test suites will additionally depend on the shared library libhiredis.so,
and libhiredis_ssl.so when SSL is enabled.

```sh
$ mkdir build; cd build
$ cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_SSL=ON ..
$ make
```

### Build options

The following CMake options are available:

* `DOWNLOAD_HIREDIS`
  * `OFF` CMake will search for an already installed hiredis (for example the
    the Debian package `libhiredis-dev`) for header files and linkage.
  * `ON` (default) hiredis will be downloaded from
    (Github)[https://github.com/redis/hiredis], built and installed locally in
    the build folder.
* `ENABLE_SSL`
  * `OFF` (default)
  * `ON` Enable SSL/TLS support and build its tests (also affect hiredis when
    `DOWNLOAD_HIREDIS=ON`).
* `DISABLE_TESTS`
  * `OFF` (default)
  * `ON` Disable compilation of tests (also affect hiredis when
    `DOWNLOAD_HIREDIS=ON`).

### Build details

The build uses CMake's (find_package)[https://cmake.org/cmake/help/latest/command/find_package.html#search-procedure] to search for a `hiredis` installation.
When building and installing `hiredis` a file called `hiredis-config.cmake` will be installed and this contains relevant information for users.

As described in the CMake docs a specific path can be set using a flag like: `-Dhiredis_DIR:PATH=${MY_DIR}/hiredis/share/hiredis`

### Running the tests

The tests and examples are located under `build/tests` after building using the
instructions above. Then run `make start` (WIP) to start a Redis cluster and then
`make test`. If you want to set up the Redis cluster manually, it should run on
localhost, where one of the nodes listens on port 30001 and has TLS disabled.
It should accept both IPv4 and IPv6 for all tests to pass. For the TLS tests,
one node is to listen on port 31001 and all nodes has to have TLS enabled.

## Quick usage

## Cluster synchronous API

### Connecting

The function `redisClusterContextInit` is used to create a `redisClusterContext`.
The function `redisClusterSetOptionAddNodes` is used to add one or many Redis Cluster addresses.
The function `redisClusterConnect2` is used to connect to the Redis Cluster.
The context is where the state for connections is kept.
The `redisClusterContext`struct has an integer `err` field that is non-zero when the connection is
in an error state. The field `errstr` will contain a string with a description of the error.
After trying to connect to Redis using `redisClusterContext` you should check the `err` field to see
if establishing the connection was successful:
```c
redisClusterContext *cc = redisClusterContextInit();
redisClusterSetOptionAddNodes(cc, "127.0.0.1:6379,127.0.0.1:6380");
redisClusterConnect2(cc);
if (cc != NULL && cc->err) {
    printf("Error: %s\n", cc->errstr);
    // handle error
}
```

### Sending commands

The function `redisClusterCommand` takes a format similar to printf.
In the simplest form it is used like:
```c
reply = redisClusterCommand(clustercontext, "SET foo bar");
```

The specifier `%s` interpolates a string in the command, and uses `strlen` to
determine the length of the string:
```c
reply = redisClusterCommand(clustercontext, "SET foo %s", value);
```
Internally, hiredis-cluster splits the command in different arguments and will
convert it to the protocol used to communicate with Redis.
One or more spaces separates arguments, so you can use the specifiers
anywhere in an argument:
```c
reply = redisClusterCommand(clustercontext, "SET key:%s %s", myid, value);
```

### Sending multi-key commands

Hiredis-cluster supports mget/mset/del multi-key commands.
The command will be splitted per slot and sent to correct Redis nodes.

Example:
```c
reply = redisClusterCommand(clustercontext, "mget %s %s %s %s", key1, key2, key3, key4);
```

### Teardown

To disconnect and free the context the following function can be used:
```c
void redisClusterFree(redisClusterContext *cc);
```
This function closes the sockets and deallocates the context.

### Cluster pipelining

The function `redisClusterGetReply` is exported as part of the Hiredis API and can be used
when a reply is expected on the socket. To pipeline commands, the only things that needs
to be done is filling up the output buffer. For this cause, two commands can be used that
are identical to the `redisClusterCommand` family, apart from not returning a reply:
```c
int redisClusterAppendCommand(redisClusterContext *cc, const char *format, ...);
int redisClusterAppendCommandArgv(redisClusterContext *cc, int argc, const char **argv);
```
After calling either function one or more times, `redisClusterGetReply` can be used to receive the
subsequent replies. The return value for this function is either `REDIS_OK` or `REDIS_ERR`, where
the latter means an error occurred while reading a reply. Just as with the other commands,
the `err` field in the context can be used to find out what the cause of this error is.
```c
void redisClusterReset(redisClusterContext *cc);
```
Warning: You must call `redisClusterReset` function after one pipelining anyway.

The following examples shows a simple cluster pipeline:
```c
redisReply *reply;
redisClusterAppendCommand(clusterContext,"SET foo bar");
redisClusterAppendCommand(clusterContext,"GET foo");
redisClusterGetReply(clusterContext,&reply); // reply for SET
freeReplyObject(reply);
redisClusterGetReply(clusterContext,&reply); // reply for GET
freeReplyObject(reply);
redisClusterReset(clusterContext);
```

## Cluster asynchronous API

Hiredis-cluster comes with an asynchronous cluster API that works with many event systems.
Currently there are adapters that enables support for libevent and Redis Event Library (ae),
but more can be added. The hiredis library has adapters for additional event systems that
easily can be adapted for hiredis-cluster as well.

### Connecting

The function `redisAsyncConnect` can be used to establish a non-blocking connection to
Redis. It returns a pointer to the newly created `redisAsyncContext` struct. The `err` field
should be checked after creation to see if there were errors creating the connection.
Because the connection that will be created is non-blocking, the kernel is not able to
instantly return if the specified host and port is able to accept a connection.
```c
redisClusterAsyncContext *acc = redisClusterAsyncConnect("127.0.0.1:6379", HIRCLUSTER_FLAG_NULL);
if (acc->err) {
    printf("Error: %s\n", acc->errstr);
    // handle error
}
```

The cluster asynchronous context can hold a disconnect callback function that is called when the
connection is disconnected (either because of an error or per user request). This function should
have the following prototype:
```c
void(const redisAsyncContext *c, int status);
```
On a disconnect, the `status` argument is set to `REDIS_OK` when disconnection was initiated by the
user, or `REDIS_ERR` when the disconnection was caused by an error. When it is `REDIS_ERR`, the `err`
field in the context can be accessed to find out the cause of the error.

You dont need to reconnect in the disconnect callback, hiredis-cluster will reconnect by itself when next command for this Redis node is handled.

Setting the disconnect callback can only be done once per context. For subsequent calls it will
return `REDIS_ERR`. The function to set the disconnect callback has the following prototype:
```c
int redisClusterAsyncSetDisconnectCallback(redisClusterAsyncContext *acc, redisDisconnectCallback *fn);
```
### Sending commands and their callbacks

In an asynchronous cluster context, commands are automatically pipelined due to the nature of an event loop.
Therefore, unlike the synchronous cluster API, there is only a single way to send commands.
Because commands are sent to Redis Cluster asynchronously, issuing a command requires a callback function
that is called when the reply is received. Reply callbacks should have the following prototype:
```c
void(redisClusterAsyncContext *acc, void *reply, void *privdata);
```
The `privdata` argument can be used to carry arbitrary data to the callback from the point where
the command is initially queued for execution.

The functions that can be used to issue commands in an asynchronous context are:
```c
int redisClusterAsyncCommand(redisClusterAsyncContext *acc,
                             redisClusterCallbackFn *fn,
                             void *privdata, const char *format, ...);
```
This function work like their blocking counterparts. The return value is `REDIS_OK` when the command
was successfully added to the output buffer and `REDIS_ERR` otherwise. Example: when the connection
is being disconnected per user-request, no new commands may be added to the output buffer and `REDIS_ERR` is
returned on calls to the `redisClusterAsyncCommand` family.

If the reply for a command with a `NULL` callback is read, it is immediately freed. When the callback
for a command is non-`NULL`, the memory is freed immediately following the callback: the reply is only
valid for the duration of the callback.

All pending callbacks are called with a `NULL` reply when the context encountered an error.

### Disconnecting

Asynchronous cluster connections can be terminated using:
```c
void redisClusterAsyncDisconnect(redisClusterAsyncContext *acc);
```
When this function is called, connections are **not** immediately terminated. Instead, new
commands are no longer accepted and connections are only terminated when all pending commands
have been written to a socket, their respective replies have been read and their respective
callbacks have been executed. After this, the disconnection callback is executed with the
`REDIS_OK` status and the context object is freed.

### Using event library *X*

There are a few hooks that need to be set on the cluster context object after it is created.
See the `adapters/` directory for bindings to *ae* and *libevent*.

## AUTHORS

This fork is based on the heronr fork (https://github.com/heronr/hiredis-vip)
and uses hiredis (https://github.com/redis/hiredis).

Hiredis-vip was originally created by vipshop (https://github.com/vipshop/hiredis-vip).

The Redis Cluster client library part in hiredis-vip was written by deep (https://github.com/deep011).

Hiredis-vip is released under the BSD license.
