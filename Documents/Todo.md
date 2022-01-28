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


Multithreaded Event
-------------------

Currently only single-threaded implementations of the event class exist

 * Free-Threaded Event class

Subscribe/Unsubscribe are still rare operations (or at least operations not
expected to happen under contention, so a lock-free singly linked list would
suffice if a free-threaded implementation of it is possible that guarantees
an unsubscribe will not prevent co-subscribers from receiving an event that
is going out at the same time)


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
