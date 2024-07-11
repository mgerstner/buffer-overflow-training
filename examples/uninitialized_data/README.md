Uninitialized Data on the Stack
===============================

This application has an issue with uninitialized data on the stack.

It's a made up scenario of a program trying to provide a _random number
service_. The program `random_svc` listens on a local UDP port, waiting for
requests. The program `random_client` can ask the service to generate a random
number by calling `rand()` a given number of times.

You can ignore the UDP communication code in this example. It's just a vehicle
for demonstrating this use case.

Note about Random Numbers
=========================

Good random data is a valuable commodity when dealing with cryptography. It is
not possible, for example, to create a secure private GPG key, SSH key or SSL
private key without high quality random numbers.

The '/dev/random' device returns such data from the kernel, that is gathered
from various sources, such as random events like interrupts, noise on the
hardware level or some dedicated hardware random number generator.  The device
'/dev/urandom' ('u' for unlimited) provides pseudo random data, that is based
on the real random data gathered from hardware.

Because there's no large amount of really unique random data available, a
typical approach in many applications is either to use data from
'/dev/urandom' or to just get a little of real random data and feed a pseudo
random number generator in userspace with it. The latter is a software
algorithm that returns randomly distributed numbers. For it to work it has to
be seeded with some truly random data, before it can be used. For the same
seed value the pseudo random number generator will always return the same
sequence of pseudo random numbers.

The corresponding C library calls are `srand()` and `rand()`. This approach is
used in the `random_svc` program found in this example.

A Note about Compiler Optimizations
===================================

The weakness modeled in this example does only work when compiler
optimizations are disabled (`-O0`). Otherwise the stack based variables are
likely to be moved into registers, which will not leak so easily onto the
stack.

In real world examples when many variables are put onto the stack this will
result in a mixture between variables put onto the stack and variables put
into registers. For some operations and calculations intermediate values are
likely to end up on the stack at times. Larger buffers always need to stay on
the stack (or the heap), so information leaks from the stack are still
interesting in real world scenarios.

Exercises
=========

- Can you make out the uninitialized data in the service implementation?
- Can we obtain the uninitialized data on the client side via some code path?
- Experiment with the uninitialized data you get, can you think of some way to
  exploit it?
- The utility `pseudo_random` will print on stdout the N'th number returned by
  the pseudo random number generator for a given seed value. You can use this
  to test an exploit of the uninitialized data issue.
