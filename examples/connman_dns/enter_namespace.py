#!/usr/bin/python3
import argparse
import os
import subprocess
import sys
from pathlib import Path

# Matthias Gerstner
# SUSE Linux GmbH
# matthias.gerstner@suse.com


def eprint(*args, **kwargs):
    kwargs['file'] = sys.stderr
    print(*args, **kwargs)


parser = argparse.ArgumentParser(description="Manages execution of a split-off network namespace for safely running connmand and exploiting it.")
parser.add_argument("--reexec", action='store_true')
parser.add_argument("--join", action='store_true')

args = parser.parse_args()

this_script = Path(__file__).absolute()
script_dir = this_script.parent
# Running D-Bus in a separate user namespace doesn't work out, because the
# user ID doesn't match the user ID in the root namespace. Lie to D-Bus about
# our real and effective user IDs using an ld-preload library.
preload = script_dir / "geteuid-preload/libgeteuid_preload.so"
instdir_conf = script_dir / "instdir.conf"

if not instdir_conf.exists():
    eprint(f"Expected location of connmand $INSTDIR in {str(instdir_conf)}")
    sys.exit(1)

instdir = Path(open(instdir_conf).read().strip())
connmand = instdir / "sbin/connmand"

if not instdir.is_dir() or not connmand.exists():
    eprint(f"{instdir} or {connmand} do not exist")
    sys.exit(1)

# export it into the environment to make example command lines from the
# README.md work as expected
os.environ["INSTDIR"] = str(instdir)

if not preload.exists():
    print("Building LD_PRELOAD helper lib\n")
    subprocess.check_call(["make"], cwd=script_dir)
    print()

ns_pidfile = instdir / "var" / "netns.pid"

if args.reexec:
    child_env = os.environ.copy()
    child_env["IN_NS_NAMESPACE"] = "1"
    child_env["LD_PRELOAD"] = str(preload)
    shell = os.environ["SHELL"]
    child_env["PS1"] = r"(network-ns) \u@\h:\w> "
    if not args.join:
        with open(ns_pidfile, 'w') as ns_fd:
            ns_fd.write(str(os.getpid()))
        # we need to mount our forked-off sysfs to reflect our network
        # namespace there correctly
        subprocess.check_call("mount -t sysfs none /sys".split())
        # fire up our new loopback device
        subprocess.check_call("ip link set up dev lo".split())
    os.execve(shell, [shell], child_env)

if "IN_NS_NAMESPACE" in os.environ:
    eprint("You are already in a private network namespace")
    sys.exit(1)

if ns_pidfile.exists():
    with open(ns_pidfile, 'r') as ns_fd:
        ns_pid = ns_fd.read().strip()

    if os.path.isdir(f"/proc/{ns_pid}"):
        print("Joining existing namespace from PID", ns_pid)
        cmdline = ["nsenter", "-n", "-U", "-m", "-t", ns_pid, f"--wd={os.getcwd()}", "--preserve-credentials", "--", this_script, "--reexec", "--join"]
        os.execve("/usr/bin/nsenter", cmdline, os.environ)
    else:
        eprint("Stale NS PID", ns_pid, "found. Removing it.")
        ns_pidfile.unlink()

if os.geteuid() == 0:
    eprint("It seems you are already root. The test setup should be created as a regular user")
    sys.exit(1)

print("Creating new network namespace")
cmdline = ["unshare", "-n", "-U", "-m", "-r", "--", this_script, "--reexec"]
os.execve("/usr/bin/unshare", cmdline, os.environ)
