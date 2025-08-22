Connection Upgrade via Heap Out Of Bounds Write in PCP
======================================================

In the summer of 2024 we reviewed the PCP software suite. A comprehensive
report about the software and some issues in it is found [on our blog][1].

A particular [heap out-of-bounds write][2] issue we found is interesting in that
none of the usual protection mechanisms help to prevent a possible exploit.

The problem is found in PCP release 6.3.0, which can be checked out from the
upstream [GitHub repository][3]. The PCP project is difficult to setup locally
in a test environment, therefore I cannot provide a fully guided practical
exploit for this example. Please refer to the source code and the blog post
explanation for the details.

References
----------

[1]: https://security.opensuse.org/2024/09/18/pcp-network-audit.html
[2]: https://security.opensuse.org/2024/09/18/pcp-network-audit.html#6-exploiting-the-heap-corruption-in-issue-5a
[3]: https://github.com/performancecopilot/pcp
