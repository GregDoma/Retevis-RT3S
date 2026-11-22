#!/usr/bin/env python3
#
# opengd77_sstv.py - save the SSTV pictures kept by an MDUV380 / RT3S running
# the SP7TEAM firmware (Menu > SSTV saves every complete picture, 8 newest
# kept), over the USB CPS link (radio on, USB cable connected, normal mode;
# close any CPS / browser tab that uses the radio first).
#
#   python3 opengd77_sstv.py -d pictures/            # one PNG per picture, x3
#   python3 opengd77_sstv.py -d pictures/ --scale 1
#   python3 opengd77_sstv.py --dump sstv.bin          # raw store (web tools open it)
#   python3 opengd77_sstv.py --file sstv.bin -d pictures/
#
# Store layout (functions/sstvStore.h): 8 slots of 40 KB at 16 MB - 384 KB.
# Slot: 32 byte header (magic "SSTV", seq, utc, rx freq / 10 Hz, rows, width,
# height, mode[12]; little endian), the picture at +0x100: 160 x 120 pixels in
# the byte order of the screen buffer (byte swapped RGB565).
#
# Requires: pyserial (pip install pyserial)

import argparse
import os
import struct
import sys
import time
import zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

CPS_ACCESS_FLASH = 1
STORE_ADDR = 16 * 1024 * 1024 - 64 * 1024 - 320 * 1024
SLOTS, SLOT_SIZE, PIXELS = 8, 40 * 1024, 0x100
WIDTH, HEIGHT = 160, 120
HEADER = struct.Struct('<IIIIHBB12s')
MAGIC = 0x56545353


def parse_header(buf):
    magic, seq, utc, freq, rows, w, h, mode = HEADER.unpack_from(buf)
    if magic != MAGIC or seq in (0, 0xFFFFFFFF) or (w, h) != (WIDTH, HEIGHT):
        return None
    return {'seq': seq, 'utc': utc if utc > 946684800 else None, 'freq': freq * 10 if freq else None,
            'rows': rows, 'mode': mode.split(b'\0')[0].decode('ascii', 'replace')}


def read_store(ser):
    from opengd77_screenshot import read_area
    raw = bytearray(b'\xff' * (SLOTS * SLOT_SIZE))
    for s in range(SLOTS):
        base = s * SLOT_SIZE
        raw[base:base + HEADER.size] = read_area(ser, CPS_ACCESS_FLASH, STORE_ADDR + base, HEADER.size)
        if parse_header(raw[base:]):
            n = WIDTH * HEIGHT * 2
            raw[base + PIXELS:base + PIXELS + n] = read_area(ser, CPS_ACCESS_FLASH, STORE_ADDR + base + PIXELS, n)
            print(f"  slot {s}: picture", file=sys.stderr)
    return bytes(raw)


def pictures(raw):
    out = []
    for s in range(SLOTS):
        base = s * SLOT_SIZE
        h = parse_header(raw[base:])
        if h:
            h['slot'] = s
            h['pixels'] = raw[base + PIXELS:base + PIXELS + WIDTH * HEIGHT * 2]
            out.append(h)
    return sorted(out, key=lambda p: -p['seq'])


def write_png(path, pixels, scale):
    raw = bytearray()
    for y in range(HEIGHT):
        line = bytearray(b'\x00')
        for x in range(WIDTH):
            i = (y * WIDTH + x) * 2
            v = (pixels[i] << 8) | pixels[i + 1]
            a, g, b = (v >> 11) & 0x1F, (v >> 5) & 0x3F, v & 0x1F
            line += bytes(((a << 3) | (a >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2))) * scale
        raw += bytes(line) * scale

    def chunk(tag, data):
        return struct.pack('>I', len(data)) + tag + data + struct.pack('>I', zlib.crc32(tag + data) & 0xFFFFFFFF)

    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n')
        f.write(chunk(b'IHDR', struct.pack('>IIBBBBB', WIDTH * scale, HEIGHT * scale, 8, 2, 0, 0, 0)))
        f.write(chunk(b'IDAT', zlib.compress(bytes(raw), 9)))
        f.write(chunk(b'IEND', b''))


def file_name(p):
    when = time.strftime('%Y-%m-%d-%H-%M-%S', time.gmtime(p['utc'])) if p['utc'] else f"nr{p['seq']}"
    return f"rt3s-sstv-{when}-{''.join(c for c in p['mode'] if c.isalnum()) or 'sstv'}.png"


def main():
    ap = argparse.ArgumentParser(description='Save the SSTV pictures kept by the radio (SP7TEAM firmware) as PNG.')
    ap.add_argument('--port', help='serial port (default: auto-detect)')
    ap.add_argument('--file', help='read a raw store saved with --dump (or by the web tools) instead of the radio')
    ap.add_argument('--dump', help='also save the raw store to this file')
    ap.add_argument('--dir', '-d', default='.', help='folder for the PNG files (default: current)')
    ap.add_argument('--scale', type=int, default=3, help='pixel scale (default 3)')
    args = ap.parse_args()

    if args.file:
        with open(args.file, 'rb') as f:
            raw = f.read()
    else:
        from opengd77_custom_data import find_port, open_port
        with open_port(args.port or find_port()) as ser:
            raw = read_store(ser)
    if args.dump:
        with open(args.dump, 'wb') as f:
            f.write(raw)

    pics = pictures(raw)
    if not pics:
        print('No SSTV pictures in the radio (firmware without the SSTV store, or none received yet).')
        return
    os.makedirs(args.dir, exist_ok=True)
    for p in pics:
        path = os.path.join(args.dir, file_name(p))
        write_png(path, p['pixels'], max(1, args.scale))
        freq = f"{p['freq'] / 1e6:.4f} MHz" if p['freq'] else '?'
        print(f"{path}  {p['mode']}  {freq}  {p['rows']}/{HEIGHT} lines")


if __name__ == '__main__':
    main()
