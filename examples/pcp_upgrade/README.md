Connection Upgrade via Heap Out Of Bounds Write in PCP
======================================================

In the summer of 2024 we reviewed the PCP software suite. A comprehensive
report about the software and some issues in it is found [on our blog][1].

[1]: https://security.opensuse.org/2024/09/18/pcp-network-audit.html

A particular [heap out-of-bounds write][2] issue we found is interesting in that
none of the usual protection mechanisms help to prevent a possible exploit.

[2]: https://security.opensuse.org/2024/09/18/pcp-network-audit.html#6-exploiting-the-heap-corruption-in-issue-5a

Due to lack of time I could not prepare a full practical guided exploit of
this issue for the training, thus we will only look at the source code and the
explanation found in the blog post to understand the issue.

The problem is found in PCP release 6.3.0, which can be checked out from the
upstream [GitHub repository][3].

[3]: https://github.com/performancecopilot/pcp
