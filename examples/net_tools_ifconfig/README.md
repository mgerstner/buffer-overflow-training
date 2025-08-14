Stack Buffer Overflow in net-tools `ifconfig`
=============================================

This vulnerability was handled in SUSE bugzilla as [bsc#1243581][1] and was
assigned CVE-2025-46836. The upstream Git repository is found on [GitHub][2].
The issue was fixed in commit 7a8f42fb20013. To retrieve the vulnerable code
you need to checkout a prior commit, there don't seem to exist appropriate
version tags to use. I looked at the code found after running:

```sh
git checkout 7a8f42fb20013^
```

[1]: https://bugzilla.suse.com/show_bug.cgi?id=1243581
[2]: https://github.com/ecki/net-tools.git

The Vulnerability
-----------------

The issue is found in function `if_readlist_proc()` in `lib/interface.c`. This
function parses content found in `/proc/net/dev` to build a global list of
network interface information. You can look at `man 5 proc_net` to find out
more about the format found in this pseudo file provided by the kernel. The
affected library code is used by the `ifconfig` when asked to display network
interface information.

The first column of each line in `/proc/net/dev` is supposed to contain the
name of a network interface. `if_readlist_proc()` uses a buffer of
`IFNAMSIZ` bytes and reads in the data found in the first column of the file
into it, without checking for a possible buffer overflow condition. The
function `get_name()` doesn't even know how big exactly the output buffer
is.

The approach used in this code likely is to globally assume that all interface
name strings are at max `IFNAMSIZ` bytes long, whichan acceptable assumption
when dealing with kernel APIs, because it is a contract between user space and
kernel that network interface names never become any longer than this.

Attack Scenario
---------------

In normal circumstances the content from `/proc/net/dev` can be trusted, since
it is provided by the kernel. There can still be situations when checking for
a possible overflow situation is a good idea. Processing data from a file
almost always is such a situation, to be prepared for unexpected surprises.

I can see mostly two attack scenarios where the assumption of trusted data
does not hold:

- another vulnerability allows an attacker to mount something else on
  `/proc/net/dev`, e.g. by performing a bind mount of an attacker controlled
  file or directory on `/proc/net/dev`. In this case the issue in net-tools could
  be exploited to further escalate privileges. If an attacker can bind mount
  arbitrary files, then there exist easier ways to escalate privileges, though
  (e.g. placing a crafted binary or script into `/usr/bin`).
- `ifconfig` is used by a privileged user on unprivileged containers, where
  non-root users can mount arbitrary data over `/proc`. This would be a bad
  idea in the first place, however.

Because of these limitations I don't think this is a very serious issue. It is
still a good example that shows that it is always better to verify one's
assumptions.

Chances to Exploit the Buffer Overflow
--------------------------------------

`IFNAMSIZ` is only 16 bytes, thus the vulnerable buffer itself is not large
enough to hold a proper attack payload. In function `if_readlist_proc` there
is another `char buf[512]` placed on the stack, which might just allow for
enough extra space to overflow to make arbitrary code execution possible. It
will depend a lot on the actual stack frame layout in the compiled binary,
when optimizations are enabled, however.

As the attack scenario for this vulnerability is quite limited I did not look
more closely into whether actual code execution can be achieved here.
