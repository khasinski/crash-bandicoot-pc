#!/usr/bin/env python3
"""Convert a --dump-vram capture (1024x512 RGB555 words) to a PNG.

    tools/scripts/vram_to_png.py vram.bin out.png [--rect X Y W H]

With --rect only that area is written, e.g. the displayed buffer
(--rect 0 0 512 240). Uses only the standard library.
"""
import argparse
import struct
import zlib


def png(path, width, height, rows):
    def chunk(kind, data):
        body = kind + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body))

    raw = b"".join(b"\x00" + row for row in rows)
    with open(path, "wb") as out:
        out.write(b"\x89PNG\r\n\x1a\n")
        out.write(chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)))
        out.write(chunk(b"IDAT", zlib.compress(raw, 9)))
        out.write(chunk(b"IEND", b""))


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("vram")
    parser.add_argument("png")
    parser.add_argument("--rect", nargs=4, type=int, metavar=("X", "Y", "W", "H"),
                        default=(0, 0, 1024, 512))
    args = parser.parse_args()
    data = open(args.vram, "rb").read()
    x0, y0, w, h = args.rect
    rows = []
    for y in range(y0, y0 + h):
        row = bytearray()
        for x in range(x0, x0 + w):
            (pixel,) = struct.unpack_from("<H", data, (y * 1024 + x) * 2)
            r, g, b = pixel & 31, (pixel >> 5) & 31, (pixel >> 10) & 31
            row += bytes(((r << 3) | (r >> 2), (g << 3) | (g >> 2), (b << 3) | (b >> 2)))
        rows.append(bytes(row))
    png(args.png, w, h, rows)


if __name__ == "__main__":
    main()
