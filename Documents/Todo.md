TODOs
=====

Multithreaded containers
------------------------

I want to have safe, portable implementations of at least:

 * Queues (single reader/writer all the way to free-threaded)
 * Lists (single reader/writer all the way to free-threaded)
 * Sets
 * Maps

The implementation doesn't have to be the very best, but they should be rock-solid,
based on built-in C++11 threading primitives and have competitive performance.


Windows Thread Pool
-------------------

It's over-engineered, slow and its concept of shutting down is missing vital
notification capabilities, meaning I still couldn't verify that the shutdown
under load is bullet proof.

 * Delete Win32 API based thread pool, port Linux thread pool to Windows


Semaphore, Gate, Latch on Windows
---------------------------------

Using Futexes via syscalls on Linux worked wonders for the performance of
the Semaphore class.

Windows has a similar capability via WaitOnAddress(), allowing potentially
much faster threading primitives than using the lame Win32 API calls.

 * Benchmark and implement Semaphore, Gate and Latch via WaitOnAddress()


DragonBox and J E A III Float and Integer Formatter
---------------------------------------------------

After switching out the earlier integer formatter (author turned fash
and calls the other side fash, typical of Russian indoctrination :D),
I am now considering moving to DragonBox.

The lynchpin is whether I can make DragonBox output strictly non-exponential
notation for floating point numbers. DragonBox also comes with an example
integer formatter (used by its float formatter) based on the J E A III
technique.

 * Rewrite DragonBox float formatter to output non-exponential notation
 * Expose its J E A III integer formatting routines for external use
 * Throw out J E A III and AMartin integer formatters
