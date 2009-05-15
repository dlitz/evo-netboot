.arch i486
.section .nbiheader, "ax"
# NBI format.  See "Draft Net Boot Image Proposal 0.3, June 15, 1997"

# Header
.int    0x1b031336      # magic
.int    0x4             # flags and length
.int    0xc00d          # load address for the 512-byte header (ds:bx format)
.int    0xc200          # execute address (cs:ip format)

# "Loading information records" follow:

# lowmem (64 sectors)
.int    0x00004104   # FIXME: tags
.int    0x00001000  # Load address
.int    0x00008000  # Image length (32768 bytes)
.int    0x00008000  # Memory length

# ram0 (64 sectors)
.int    0x00004104   # FIXME: tags
.int    0x01000000  # Load address
.int    0x00008000  # Image length (32768 bytes)
.int    0x00008000  # Memory length

# bzImage (8192 sectors)
.int    0x04004104  # FIXME: tags (last entry)
.int    0x01080000  # Load address
.int    0x00400000  # Image length (4194304 bytes)
.int    0x00400000  # Memory length

.fill   435, 1, 0x00

hdr_entry_point:
    # jump far, absolute, to immediate address 08:01000000
    ljmp $0x8, $0x01000000

.fill   6, 1, 0x00
