TODOs
=====

Multithreaded containers
------------------------

I want to have safe, portable implementations of at least:

 * Queues (single reader/writer all the way to free-threaded)
 * Lists (single reader/writer all the way to free-threaded)
 * Sets
 * Maps

The implementation doesn't have to be the very best, but it should be rock-solid,
based on built-in C++11 threading primitives and have competitive performance.


Command Line Parser
-------------------

A DOM-style command line parser that can be used as the basis for either accepting
command line arguments directly or for building a getopt()-style library.

  * Parse command line from whole string
  * Parse command line from argc, argv
  * Windows and Linux style argument passing
  * Build command line by appending arguments
