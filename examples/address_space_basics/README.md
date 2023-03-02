Example Program For Exploring Address Space Basics on a Linux System
====================================================================

Compile and run the Program
---------------------------

    $ make
    # don't enter a newline once it started, keep the program running
    ./simple

Compare the Addresses
---------------------

Compare the addresses printed by the program on stdout with the address
ranges the kernel knows about. In another shell type:

    $ cat /proc/`pidof simple`/maps

Can you find out to which memory areas the distinct types of data and
functions belong to?
