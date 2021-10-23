# librfdict

Simple C library for string to integer mapping.

This library implements a self-balancing red-black tree that has string keys and long integer values.  For simplicitly, the library only includes the insertion and find operations, and does not provide delete key or alter existing record functions.

This is intended for the limited but common case where you need to assign unique integers to different string keys and have a way of efficiently querying the mapping of string key to integer at any time.  The self-balancing red-black tree that is used internally guarantees efficient access at all times.

The whole library is contained within the `rfdict.c` and `rfdict.h` files, which have no external dependencies.  Two testing programs are also included in this distribution.
