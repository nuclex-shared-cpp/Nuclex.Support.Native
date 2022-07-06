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


Windows Thread Pool
-------------------

Microsoft's thread pool is over-engineered, slow and its concept of shutting down
is missing vital notification capabilities, meaning I still couldn't verify that
a shutdown under load is bullet proof.

 * Delete Win32 API based thread pool, port Linux thread pool to Windows


Semaphore, Gate, Latch on Windows
---------------------------------

Using Futexes via syscalls on Linux worked wonders for the performance of
the Semaphore class.

Windows has a similar capability via WaitOnAddress(), allowing potentially
much faster threading primitives than using the lame Win32 API calls.

 * Benchmark and implement Semaphore, Gate and Latch via WaitOnAddress()
