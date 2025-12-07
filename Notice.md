Nuclex.Support.Native Attribution
=================================

This library is licensed under the Apache License 2.0,
http://www.apache.org/licenses/


You can:
--------

- Use this library in closed source and commercial applications.
- Distribute this library's unmodified source code.
- Distribute binaries compiled from the unmodified source code.

- Modify the source code of this library and keep the changes to yourself.
- Modify the source code of this library and publish the changes,
  so long as you make it very clear that, and how, you changed the code.


You can not:
------------

- Put this library under a different license or
- Sell access to this library's code

- Sue anyone for issues you have with this code.

- Pretend that you wrote this
- Use the names of any of this library's authors to promote your own work.


Attribution
===========

If you distribute binaries of this library, you should include this license
file somewhere in your documentation or other legal text.

A mention of your product's use of this library as well as of the embedded
third-party libraries in your splash screen, credits or such would be nice,
but is not required.


Example Attribution Text
------------------------

Uses supporting code from the Nuclex.Support.Native library, written by
Markus Ewald and licensed under the terms of the Apache License 2.0
(http://www.apache.org/licenses/)

Nuclex.Support.Native also embeds additional libraries, belonging to their
respective owners and used according to their respective licenses:

  * The James Edward Anhalt III integer formatter under the MIT license
  * Junekey Jeon's DragonBox float formatter under the Apache 2.0 license
  * Daniel Lemire's fast_float parsing library under the Apache 2.0 license
  * Ulf Adams' float parser from the Ryu library under the Apache 2.0 license
  * Cameron Desrochers' Multi-threaded queues under the Boost Software license


Third-Party Code
================

This library also embeds carefully selected third-party code which falls
under its own licenses, listed below


James Edward Anhalt III Integer Formatter: MIT License
------------------------------------------------------

A modified version of James Edward Anhalt III.'s integer printing algorithm
is used to convert integral values into strings. The original code is licensed
under MIT license (https://mit-license.org/)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


DragonBox: Apache License 2.0
-----------------------------

Uses code from Junekey Jeon's DragonBox reference implementation to convert
floating point values into string independent of the system locale. Licensed
under the Apache 2.0 license (http://www.apache.org/licenses/LICENSE-2.0)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


FastFloat: Apache License 2.0
-----------------------------

Uses code from Daniel Lemire's fast_float number parsing library to convert
strings values into numbers independent of the system locale. Licensed
under the Apache 2.0 license (http://www.apache.org/licenses/LICENSE-2.0)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


Ryu String to Float Parser: Apache License 2.0
----------------------------------------------

Code from the Ryu library is used to convert strings to floating point values
independent of the system locale. It is written by Ulf Adams and licensed under
the Apache 2.0 license (http://www.apache.org/licenses/LICENSE-2.0)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.


MoodyCamel Lock-Free Unbounded Concurrent Queue
-----------------------------------------------

The Linux thread pool code internally uses the unbounded concurrent queue code
by Cameron Desrochers, included in a subdirectory. It is license under
the Boost Software License (https://www.boost.org/users/license.html)

If you distribute binaries compiled from this library, you do not have to
do anything, but an acknowledgement of the original author would be nice.
