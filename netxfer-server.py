import socket
import IN
import struct
import sys

def decode_zstr(zs):
    """Decode a NUL-terminated string"""
    return zs[:zs.index("\0")]

def encode_dhcp_message(msg):
    # Encode options
    raw_options = ["\x63\x82\x53\x63"]
    for opt_type in sorted(msg['options'].keys()):
        opt_value = msg['options'][opt_type]
        raw_options.append(chr(k))
        raw_options.append(chr(len(opt_value)))
        raw_options.append(opt_value)
    raw_options.append("\xff")
    raw_options = "".join(raw_options)

    if len(raw_options) < 64:
        raw_options = struct.pack("!64s", raw_options)

    field_names = (
        'op', 'htype', 'hlen', 'hops', 'xid', 'secs', 'flags', 'ciaddr',
        'yiaddr', 'siaddr', 'giaddr', 'chaddr', 'sname', 'file')
    msg_fields = []
    for field_name in field_names:
        msg_fields.append(msg[field_name])
    raw_msg = struct.pack("!BBBBLHH4s4s4s4s16s64s128s", *tuple(msg_fields))
    raw_msg += raw_options
    return raw_msg

def decode_dhcp_message(raw_msg):
    field_names = (
        'op', 'htype', 'hlen', 'hops', 'xid', 'secs', 'flags', 'ciaddr',
        'yiaddr', 'siaddr', 'giaddr', 'chaddr', 'sname', 'file')
    msg_fields = struct.unpack("!BBBBLHH4s4s4s4s16s64s128s", raw_msg[:236])
    raw_options = raw_msg[236:]
    msg = {}
    for i, field_name in enumerate(field_names):
        msg[field_name] = msg_fields[i]
    msg['sname'] = decode_zstr(msg['sname'])
    msg['file'] = decode_zstr(msg['file'])
    if len(msg['chaddr']) < msg['hlen']:
        raise ValueError("Invalid 'hlen' value (%d)" % (msg['hlen'],))
    msg['chaddr'] = msg['chaddr'][:msg['hlen']]
    #msg['options'] = raw_options

    # Parse DHCP options
    if not raw_options.startswith("\x63\x82\x53\x63"):  # magic
        raise ValueError("Options field does not contain options magic")

    msg['options'] = {}
    p = 4
    while p < len(raw_options):
        opt_type = ord(raw_options[p])
        if opt_type == 0:
            # padding
            p += 1
            continue
        elif opt_type == 255:
            # end of options
            p += 1
            break
        if p+1 >= len(raw_options):
            raise ValueError("Unexpected end of DHCP options field")
        opt_length = ord(raw_options[p+1])
        p += 2
        if p+opt_length >= len(raw_options):
            raise ValueError("Unexpected end of DHCP options field")
        opt_value = raw_options[p:p+opt_length]
        p += opt_length
        msg['options'][opt_type] = opt_value
    else:
        raise ValueError("Unexpected end of DHCP options field")

    return msg

def decode_tftp_packet(raw_pkt):
    pkt = {}
    (opcode,) = struct.unpack("!H", raw_pkt[:2])
    if opcode in (1, 2):    # RRQ/WRQ
        pkt['op'] = {1: "RRQ", 2: "WRQ"}[opcode]
        p = 2
        q = raw_pkt.index("\0", p)
        pkt['filename'] = raw_pkt[p:q]
        p = q+1
        q = raw_pkt.index("\0", p)
        pkt['mode'] = raw_pkt[p:q]
        p = q+1
        # FIXME: extend this to handle all options (see RFC 2347)
        if p < len(raw_pkt) and raw_pkt[p:].startswith("blksize\0"):
            p += len("blksize\0")
            q = raw_pkt.index("\0", p)
            pkt['blksize'] = int(raw_pkt[p:q])
    elif opcode == 3:       # DATA
        pkt['op'] = 'DATA'
        (pkt['blocknum'],) = struct.unpack("!H", raw_pkt[2:4])
        pkt['data'] = raw_pkt[4:]
    elif opcode == 4:       # ACK
        pkt['op'] = 'ACK'
        (pkt['blocknum'],) = struct.unpack("!H", raw_pkt[2:4])
    elif opcode == 5:       # ERROR
        pkt['op'] = 'ERROR'
        (pkt['errorcode'],) = struct.unpack("!H", raw_pkt[2:4])
        p = 4
        q = raw_pkt.index("\0", p)
        pkt['errmsg'] = raw_pkt[p:q]
    else:
        raise ValueError("Unrecognized TFTP packet opcode %d" % (opcode,))
    return pkt

def encode_tftp_packet(pkt):
    retval = []
    if pkt['op'] in ('RRQ', 'WRQ'):
        retval.append({'RRQ': "\x00\x01", 'WRQ': '\x00\x02'}[pkt['op']])
        retval.append(pkt['filename'])
        retval.append("\x00")
        retval.append(pkt['mode'])
        retval.append("\x00")
        # FIXME: extend this to handle all options (see RFC 2347)
        if "blksize" in pkt:
            retval.append("%d" % pkt['blksize'])
            retval.append("\x00")
    elif pkt['op'] == 'DATA':
        retval.append(struct.pack("!HH", 3, pkt['blocknum']))
        retval.append(pkt['data'])
    elif pkt['op'] == 'ACK':
        retval.append(struct.pack("!HH", 4, pkt['blocknum']))
    elif pkt['op'] == 'ERROR':
        retval.append(struct.pack("!HH", 5, pkt['errcode']))
        retval.append(pkt['errmsg'])
        retval.append("\x00")
    elif pkt['op'] == 'OACK':
        retval.append("\x00\x06")
        for k in sorted(pkt['options']):
            retval.append(k)
            retval.append("\0")
            retval.append(pkt['options'][k])
            retval.append("\0")
    else:
        raise ValueError("Unrecognized 'op' value %r" % (pkt['op'],))
    return "".join(retval)

def format_haddr(haddr):
    """Format hardware address for display"""
    return ":".join("%02x" % ord(c) for c in haddr)

def serve_bootp(iface, client_v4addr, server_v4addr, gateway_v4addr):
    ## BOOTP ##
    skt = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    skt.bind(('', 10067))
    skt.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    skt.setsockopt(socket.SOL_SOCKET, IN.SO_BINDTODEVICE, iface + "\0")

    # Get BOOTREQUEST
    (raw_msg, addr) = skt.recvfrom(65535)
    msg = decode_dhcp_message(raw_msg)
    assert msg['op'] == 1   # BOOTREQUEST
    print "Got BOOTREQUEST from %r: %s" % (addr, format_haddr(msg['chaddr']))

    # Send BOOTREPLY
    msg['op'] = 2   # BOOTREPLY
    msg['yiaddr'] = "".join(chr(int(c)) for c in client_v4addr.split("."))
    msg['siaddr'] = "".join(chr(int(c)) for c in server_v4addr.split("."))
    msg['giaddr'] = "".join(chr(int(c)) for c in gateway_v4addr.split("."))
    msg['file'] = "bootp.bin"
    raw_msg = encode_dhcp_message(msg)
    skt.sendto(raw_msg, ('255.255.255.255', 10068))

    skt.close()

def serve_tftp(filename):
    ## TFTP
    skt = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    skt.bind(('', 10069))

    raw_pkt, addr = skt.recvfrom(65535)
    pkt = decode_tftp_packet(raw_pkt)
    assert pkt['op'] == 'RRQ'

    blocksize = 512

    #if 'blksize' in pkt:
    #    blocksize = pkt['blksize']
    #    # Send OACK (option acknowledge)
    #    raw = encode_tftp_packet({'op': 'OACK', 'options': {'blksize': "%d" % (blocksize,)}})
    #    skt.sendto(raw, addr)
    #    # FIXME: Wait for ACK
    #else:
    #    blocksize = 512
    #
    ## DEBUG FIXME
    #raw_pkt, addr = skt.recvfrom(65535)
    #pkt = decode_tftp_packet(raw_pkt)
    #assert pkt['op'] == 'RRQ'

    f = open(filename, "rb")
    last_blocksize = None
    blocknum = 0
    state = "NEXTBLOCK"
    while state != "DONE":
        if state == 'NEXTBLOCK':
            blocknum += 1
            block = f.read(blocksize)
            state = 'SENDDATA'
        elif state == 'SENDDATA':
            # Send DATA packet
            print "Sending block #%d (%d bytes) to %r" % (blocknum, len(block), addr)
            pkt = {
                'op': 'DATA',
                'blocknum': blocknum,
                'data': block,
            }
            raw_pkt = encode_tftp_packet(pkt)
            skt.settimeout(None) # No timeout
            skt.sendto(raw_pkt, addr)
            state = 'WAITFORACK'
        elif state == 'WAITFORACK':
            # Wait for ACK
            skt.settimeout(2.0) # 2-second timeout
            try:
                raw_pkt, pkt_addr = skt.recvfrom(65535)
            except socket.timeout:
                print "TIMEOUT"
                state = "SENDDATA"
                continue
            if pkt_addr != addr:
                continue
            pkt = decode_tftp_packet(raw_pkt)
            if pkt['op'] != 'ACK':
                continue
            if pkt['blocknum'] != blocknum:
                continue
            if len(block) < blocksize:
                state = 'DONE'
            else:
                state = 'NEXTBLOCK'

if __name__ == '__main__':
    serve_bootp(iface="eth0", client_v4addr="192.168.1.2", server_v4addr="192.168.1.1", gateway_v4addr="192.168.1.1")
    serve_tftp(sys.argv[1])
