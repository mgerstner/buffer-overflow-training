Stack Overflow Vulnerabilities in the sngrep SIP Monitoring Tool
================================================================

`sngrep` is a TUI monitoring utility for SIP Voice over IP calls. It is
implemented in C and the code is pretty lax in processing the arbitrary
untrusted network data that is captured via the libpcap library (the backend of
tools like `tcpdump`).

A stack overflow vulnerability in `sngrep` was reported in April 2024 in
[bnc#1222594][1], CVE-2024-3120. There is an upstream [bugfix pull request][2]
on GitHub that fixes the vulnerability. Actually there are two commits found
in the codebase that fix stack overflow issues: commit [f3f8ed8ef3][3] and
commit [dd5fec9273][4].

These stack overflow issues are interesting in their nature, on the one hand,
because of the carelessness with which the untrusted network data is dealt
with. On the other hand, because of real world limitations of actually
exploiting these issues for arbitrary code execution. Only one of the two
vulnerabilities can actually be fully exploited, but only by overcoming a few
extra obstacles.

For the training we will hit each obstacle step by step, analyze it, and find
ways around it. At the end of this document the obstacles and solutions to
them are documented, should you not be able to find your way around them.

Affected Source
---------------

The Git repository is found on GitHub <https://github.com/irontec/sngrep>.
Upstream version 1.8.1 contains bugfixes for both stack overflow issues, so
for our example we will need to checkout Git tag `v1.8.0`:

    git clone https://github.com/irontec/sngrep
    cd sngrep
    git checkout v1.8.0

Building sngrep
---------------

For building the affected version you will need devel packages for `libpcap`
and possibly also for `ncurses`:

    zypper in libpcap-devel
    zypper in ncurses-devel

For building we need to generate the autotools based buildsystem and build
using the classic configure/make triplet. For a successful build the
`--enable-unicode` switch also needs to be passed to the configure script. For
specifying custom CFLAGS, the CFLAGS environment variable can be passed to
configure as well. In the following example we will install the build
artifacts directly into the source tree, which is enough for our exploit
purposes.

    cd sngrep
    autoreconf
    ./bootstrap.sh
    # the CFLAGS here produces an optimize build, similar to an actual release
    # build, but with included debugging symbols
    CFLAGS="-O2 -g" ./configure --enable-unicode
    make
    make install DESTDIR=$PWD/install
    # make sure we get an executable stack for exploiting the issue
    execstack -s ./install/usr/local/bin/sngrep

For creating builds with different compiler settings you need to rerun the
configure step using the appropriate `CFLAGS` environment variable, then `make
clean` and repeat the rest of the install steps.

Running sngrep
--------------

The `sngrep` tool is capturing raw network packets from Ethernet network
devices in the system. This is a privileged operation, that either needs full
root privileges, or the `CAP_NET_RAW` capability. For our testing purposes we
don't want to run a vulnerable utility on a life network. Instead we are
employing container techniques and split-off a separate Linux network
namespace. This separate network namespace will only have an isolated loopback
network device, not connected to any other components in the system.

This example directory contains a helper script `enter_namespace.py` that
either creates such a namespace setup, or joins an existing one. You will need
at least two shells in this separate network environment for testing exploits.
All further exploit operations have to happen in a `(network-ns)` sub-shell.
You also need to make the installation directory known to the script
beforehand:

    # in examples/phone2overflow
    phone2exploit $ echo path/to/sngrep/install >instdir.conf
    phone2exploit $ ./enter_namespace.py
    Creating new network namespace
    (network-ns) $ ip addr
    1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group
    default qlen 1000
        link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
        inet 127.0.0.1/8 scope host lo
           valid_lft forever preferred_lft forever
        inet6 ::1/128 scope host proto kernel_lo
           valid_lft forever preferred_lft forever

Within the separate namespace you can run your custom built `sngrep` like
this:

    (network-ns) $ ~/sngrep/install/usr/local/bin/sngrep -q -d lo -N

This will disable all the terminal output of the tool and restrict listening
to the loopback device. For passing network packets to it you can simply use
the netcat utility. It can be installed this way:

    root # zypper in netcat-openbsd

The netcat utility can create all kinds of network connections and pass data.
The `sngrep` is not picky about the data it processes. The IP port is not
inspected and neither are any protocol types, thus any IP packets for UDP or
TCP are captured and processed further. You can check your basic network setup
is working like follows. In one terminal run `sngrep` in `gdb` and setup a
breakpoint in the `parse_packet()` function. Any captured packet will go
through there:

    (network-ns) $ gdb --args ~/sngrep/install/usr/local/bin/sngrep -q -d lo -N
    (gdb) b parse_packet
    (gdb) r

In a second terminal, simply send arbitrary data over the loopback device
using netcat:

    (network-ns) $ nc -4 -u localhost 1234 </etc/os-release

We are passing the `-u` and `-4` switches to netcat here, for using IPv4 and
the UDP protocol, which is the simplest approach to use for the purposes of
our exploit attempts.

Approaching the Exploit
-----------------------

First try to get a feeling to the code we are dealing with here. The core of
the packet processing logic involving the two stack buffer overflows is
rather small. The function `parse_packet()` mentioned above is the central
entry point for all packets that are captured. The buffer overflow in
`sip_validate_packet()` only happens on the TCP packet parsing path in the
following call stack:

    #0 sip_validate_packet()
    #1 capture_packet_reasm_tcp()
    #2 parse_packet()

The buffer overflow in `sip_get_callid()` happens for both UDP and TCP packet
parsing in the following call stack:

    #0 sip_get_callid()
    #1 sip_check_packet()
    #2 capture_packet_parse()
    #3 parse_packet()

For letting `sngrep` actually reach these locations, the network data must
match some regular expressions that are setup in the `sip_init()` function.
For these two stack buffer overflows the following regular expressions are
relevant.

    # This is the only check that needs to pass for reaching the overflow in sip_get_callid()
    calls.reg_callid: "^(Call-ID|i):[ ]*([^ ]+)[ ]*\r$"

    # These two checks need to pass for reaching the overflow in sip_validate_packet()
    calls.reg_valid: "^([A-Z]+ [a-zA-Z]+:|SIP/2.0 [0-9]{3})"
    calls.reg_cl, "^(Content-Length|l):[ ]*([0-9]+)[ ]*\r$"

As we can see from these regular expressions, the `sngrep` tool parses
arbitrary binary packets in textual form. Using the netcat utility is enough
for sending out payloads that match these patterns and also contain exploit
data.

Before actually exploiting anything you should look more carefully at the code
involving the stack buffer overflows. Check whether attempting the exploit has
any chances of success in the first place.

For constructing suitable exploits, you can simply use shell utilities like
`cat` and `echo`. The following example sends a payload that should produce a
match in the `sip_get_callid()` regular expression check.

    (network-ns) $ echo -e -n "Call-ID: <exploit-goes-here> \r" >payload
    (network-ns) $ nc -4 -u localhost 1234 <payload

When sending this payload over the loopback device, the debugger running
`sngrep` should hit the `sip_get_callid()` function, and the match should
contain the `<exploit goes here>` string. Note that because of the nature of
the regular expression the exploit data cannot contain any space character
(0x20).

You can now place the prefix and suffix strings for matching the regular
expression into separate files, and can use them for trying out different
exploit payloads:

    echo -e -n "Call-ID: " >payload.prefix
    echo -e -n " \r" >payload.suffix
    cat payload.prefix exploit.bin payload.suffix >payload

Obstacles and their Solutions
-----------------------------

### `sip_validate_packet()` is not Exploitable

Check the `regexec(&calls.reg_valid, [...]` call in the function and the
`strncpy()` that follows it. Which regex capture group is copied into the
target buffer here? Why is that making a code execution exploit impossible?

### `sip_get_callid()` Overflow does not even Crash `sngrep`?

When attempting to trigger the buffer overflow in `sip_get_callid()` you may
notice that not only the exploit does not work, but the `sngrep` process seems
completely unimpressed, it is not even crashing.

This situation stems from the kind of compiler optimization / reordering that
happens here when `sngrep` is compiled with `-O2` optimizations. In the
debugger when breaking execution in `sip_check_packet()`, inspect the
addresses of the stack based buffers `callid`, `xcallid` and `payload` and
their sizes. As it turns out the compiler places the large `payload` buffer
first on the stack, only then the smaller `callid` and `xcallid` buffers. This
currently happens on openSUSE Tumbleweed using `gcc` version `13.3.0`.

When this happens, then the overflow takes place, but we can at most supply
`MAX_SIP_PAYLOAD` bytes (10240 bytes). When overflowing the `callid` buffer
then we don't reach the actual stack frame administrative data, but only the
differently placed `payload` buffer. So we are able to overwrite our own
payload data, which doesn't make much sense. It doesn't even allow to DoS the
process. Lucky you, `sngrep`!

To overcome this limitation (for learning purposes) we can create a debug
build of `sngrep` by passing `CFLAGS="-O0 -g"`. When doing this, then the
order of the stack based buffers is as it appears in the code, and the exploit
becomes possible.

### `sip_get_callid()` is a Zero Byte Terminated Buffer Overflow

It is not that obvious at first, because the regular expression handling code
is so careless and not checking and target buffer lengths at all. However,
although the `regexec()` call also does not receive and source buffer length,
it is a function that expects properly zero-terminated strings. Thus the
regular expression will only consider payload content up to the first zero
byte. This limits our exploit payload considerably. Our execve snippet from
the `exec_snippet` example folder is not suitable for exploiting this kind of
vulnerability. The folder does contain alternative `nozeroes.S` assembly,
though, which is designed in a way that it does not contain any zero bytes in
the machine code at all.

Even with this advanced machine code we are still faced with further problems
from this zero-byte-terminated buffer overflow. The return addresses placed
into the exploit buffer by the `gen_exploit.py` helper script will also
contain zero bytes, at least on 64-bit architectures, where the two upper
bytes by design have to be zero.

This leaves us only with one last chance to exploit this: We can only include
exactly one return address in the exploit payload, which will be missing the
upper two zero bytes. And this single return address will have to perfectly
overwrite the existing return address in the stack frame of the
`sip_check_packet()` function call. This means the attack is pretty limited:
We can still perform the NOP slide, i.e. the return address does not need to
have to a perfect match. But the location of the incomplete return address in
the exploit payload needs to be perfect, because there is only one incomplete
copy of it available. Even this attack only works, because the legitimate
return address present on the stack frame already needs to contain the two
necessary upper zero bytes.

To construct such an exploit payload, we need to tune the invocation of the
`gen_exploit.py` script, to generate a structure with only a single return
address at the end of the buffer.

    ./gen_exploit.py -s 1080 -c ../exec_snippet/nozeroes.bin -a 0x7ffff75fa640 -o --truncated-return-address >exploit.bin

To learn of the correct size of the exploit buffer you need to utilize the
debugger in the `sip_check_packet()` function. Check the `info frame` output,
here the stored return address is displayed. Inspect the stack frame using `x
/50gx <address-of-frame>` and similar commands to find out at which location
exactly the return address is stored in the stack frame. Now you need to
calculate the distance between the address of the `callid` buffer and the
return address. The exploit buffer needs to match this range exactly for
hitting the return address.

### The Exploit Payload Overflows and Runs, but then Crashes?!

Even after overcoming the issues regarding zero byte termination and even when
correctly hitting the return address, something strange seems to happen. An
illegal instruction, or a segmentation fault. Checking the debugger upon
returning to the overflow data looks correct at first, but then some strange
instructions show up.

The reason for this is found in the `if (!msg = msg_create())` line that
follows our stack buffer overflow. `msg` is a stack based pointer variable. Of
all things, this variable is placed directly within our exploit code! The
assignment will break our code and make the exploit unusable.

To avoid this situation, the exploit code needs to be placed at the end of the
exploit buffer, but some hundred bytes earlier. This way this variable
assignment will no longer break our exploit code. It will make the NOP slide
somewhat shorter, but only by a bit.

You can achieve this by passing the `--shift-exploit` parameter to
`gen_exploit.py`:

    ./gen_exploit.py -s 1080 -c ../exec_snippet/nozeroes.bin -a 0x7ffff75fa640 -o --truncated-return-address --shift-exploit 128

Lessons Learned
---------------

- This example shows how zero-terminated buffer overflows are a lot harder to
  exploit than other kinds.
- The `sngrep` tool lenient with the untrusted network data it processes. It
  should limit the type of packets it parses more diligently. It should
  discard any packets that don't carry a proper SIP header, and should also
  filter packets containing any non-ASCII characters.
- This character based processing of potentially binary data should be
  avoided.
- The lack of buffer size checks should speak for itself.

References
----------

- [SUSE Bugzilla bug for sngrep stack buffer overflow][1]
- [GitHub pull request addressing one of the stack buffer overflows][2]
- [Bugfix for a stack buffer overflow in the `sip_get_callid()` function][3]
- [Bugfix for a stack buffer overflow in the `sip_validate_packet()` function][4]

[1]: https://bugzilla.suse.com/show_bug.cgi?id=1222594
[2]: https://github.com/irontec/sngrep/pull/480/commits/f229a5d31b0be6a6cc3ab4cd9bfa4a1b5c5714c6
[3]: https://github.com/irontec/sngrep/commit/f3f8ed8ef38748e6d61044b39b0dabd7e37c6809
[4]: https://github.com/irontec/sngrep/commit/dd5fec92730562af6f96891291cd4e102b80bfcc
