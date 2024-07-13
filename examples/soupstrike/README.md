A Real Stack Overflow Vulnerability in Gnome's libsoup Library
==============================================================

This was a vulnerability handled in [bsc#1052916][1], CVE-2017-2885.

[1]: https://bugzilla.suse.com/show_bug.cgi?id=1052916

You can read the bug description for details on what went wrong here. The
buffer in question is found in _soup-body-input-stream.c_, function
`soup_body_input_stream_read_chunked()` and its name is `char metabuf[128]`.

This overflow is fully remote exploitable, where it not for the various
protection mechanisms under the hood.

Exercise
========

First try to understand the vulnerable `metabuf` construct in the code and a
bit about the involved HTTP chunked encoding. The report in the linked
bug is pretty detailed about this.

Can you manage to exploit this issue? Try to exploit the issue using an
attacker buffer created via `gen_exploit.py`. Keep in mind the following
aspects:

- libsoup ships an example http server called _simple-httpd_. By compiling it
  we have a suitable program to test an attack against.
- The upstream repo is at https://github.com/GNOME/libsoup.git. Version tag
  `2.58.1` is the most recent version that contains the vulnerability.
- To trigger the issue you can use the accompanying script `hit-the-soup.py`.
  It basically creates an HTTP header that enables chunked encoding first and
  the next line of text can already overflow the target buffer.
- The exploit code must not contain the sequence "\r\n", because that would
  terminate the copying operation.

The following sections explain how to correctly build and safely start the
vulnerable web server for testing exploits against it.

Building libsoup
----------------

To build libsoup with disabled protection mechanisms follow these steps:

```sh
# you may need to install these additional dependencies
$ sudo zypper in sqlite3-devel gtk-doc libxml2-devel intltool

# clone and checkout the vulnerable libsoup codebase
$ git clone https://github.com/GNOME/libsoup.git
$ cd libsoup
$ git checkout 2.58.1

# create the autotools build system
$ ./autogen.sh

# build libsoup with debugging symbols and disabled stack protector
$ CFLAGS="-g -fno-stack-protector" ./configure
$ make

# mark the simple-httpd binary with `execstack -s` to get an executable stack.
$ execstack -s examples/.libs/simple-httpd
```

Exploiting the Issue
--------------------

Don't run the simple-httpd executable directly, because this might be using
system libraries instead of the ones we compiled locally. The wrapper script
in `examples/simple-httpd` performs the necessary setup to make sure that
the locally built and vulnerable libraries will be used.

The following steps will run the sample web server in an isolated environment:

```sh
# split-off a network namespace to avoid having the vulnerable daemon
# listening on a live network
$ unshare -U -n -r
# set up the loopback device in the new network namespace
$ ip addr add 127.0.0.1/8 dev lo
$ ip link set up dev lo

# enter a shell without protection mechanisms
(netns) $ setarch `uname -m` -R /bin/bash

# run the server on a fixed port
(netns-exploit) $ ./examples/simple-httpd -p 4711
```

---
**NOTE:**

The network namespace used here is part of the Linux container isolation
techniques. The shell started via the `unshare` command is attached to a
split-off networking stack that has no access to the existing network devices
of the system. Only other processes that join this network namespace will be
able to communicate over the network with the simple-httpd started this way.

---

In another shell we attach to the newly created network namespace and test
whether the http server works:

```sh
# join the network namespace simple-httpd is living in now
$ nsenter -t `pidof simple-httpd` -U -n --preserve-credentials

# the server serves files from the CWD it runs in, thus we should be able to
# fetch the NEWS file from the libsoup checkout
(netns) $ curl localhost:4711/NEWS
[...]
```

In this client shell we can now attempt to trigger a full exploit. The
`hit-the-soup.py` helper script is able to create a suitably prepared exploit
that we can send to the simple-httpd via netcat:

```sh
$ ./examples/soupstrike/hit-the-soup.py /path/to/exploit.bin | nc localhost 4711
```

The exploit needs to be constructed using the `gen_exploit.py` script and the
proper return address for `metabuf`.

For running `gdb` with `simple-httpd` it is best to attach to the running
process, since we are only executing a wrapper script for `simple-httpd` which
cannot be started directly in `gdb`. Use `gdb -p $(pidof simple-httpd)`
instead.
