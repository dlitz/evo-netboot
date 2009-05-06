#!/usr/bin/python
import sys
import re
import binascii

#infile = open(sys.argv[1], "r")
infile = sys.stdin
outfile = sys.stdout
r = re.compile(r"^([0-9a-f]{8}):([0-9a-f]*);([0-9a-f]{4})")
state = "START"
while True:
    rawline = infile.readline()
    if not rawline:
        raise EOFError
    line = rawline.rstrip()
    if state == "START":
        if "== FLASH START ==" in rawline:
            cksum = 0
            addr = 0
            state = "HEADER"
        else:
            pass
    elif state == "HEADER":
        if line == ";0000":
            state = "READ"
        else:
            pass
    elif state == "READ":
        if not line:
            continue
        m = r.search(line)
        if not m:
            if "== FLASH DONE ==" in line:
                state = "DONE"
                break
            else:
                raise ValueError("unable to read line: %r" % (line,))
        line_addr = int(m.group(1), 16)
        bytes = m.group(2)
        line_cksum = int(m.group(3), 16)
        bytes = bytes.replace(" ", "")
        if len(bytes) != 2*16:
            raise ValueError("unable to read line: %r" % (line,))
        rawbytes = binascii.a2b_hex(bytes)
        for b in rawbytes:
            cksum = (((cksum << 1) | (cksum >> 31)) ^ ord(b)) & 0xffffffff
        if line_cksum != (cksum & 0xffff):
            raise ValueError("checksum error on line: %r" % (line,))
        if line_addr != addr:
            raise ValueError("address error on line: %r" % (line,))
        addr += len(rawbytes)
        outfile.write(rawbytes)
