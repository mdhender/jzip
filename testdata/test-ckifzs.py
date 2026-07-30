#!/usr/bin/env python3

import subprocess
import sys
import tempfile
from pathlib import Path


def chunk(identifier: bytes, data: bytes) -> bytes:
    encoded = identifier + len(data).to_bytes(4, "big") + data
    return encoded + (b"\0" if len(data) % 2 else b"")


def save(*chunks: bytes, trailing: bytes = b"") -> bytes:
    body = b"".join(chunks)
    return b"FORM" + (len(body) + 4).to_bytes(4, "big") + b"IFZS" + body + trailing


IFHD = (119).to_bytes(2, "big") + b"880429" + (0xBF44).to_bytes(2, "big") + bytes(
    [0, 0x75, 0x90]
)
CMEM = b"\0\x0a"
DUMMY_FRAME = bytes(8)


def fixture(ifhd: bytes = IFHD, stks: bytes = DUMMY_FRAME, *extra: bytes) -> bytes:
    return save(chunk(b"IFhd", ifhd), chunk(b"CMem", CMEM), chunk(b"Stks", stks), *extra)


def check(ckifzs: str, directory: Path, name: str, data: bytes, expected: int, text: str) -> None:
    path = directory / name
    path.write_bytes(data)
    result = subprocess.run([ckifzs, str(path)], capture_output=True, text=True, check=False)
    if result.returncode != expected or text not in result.stdout:
        raise AssertionError(
            f"{name}: expected status {expected} and {text!r}\n"
            f"status: {result.returncode}\nstdout:\n{result.stdout}\nstderr:\n{result.stderr}"
        )


def main() -> int:
    if len(sys.argv) != 2:
        print(f"usage: {sys.argv[0]} CKIFZS", file=sys.stderr)
        return 2

    ckifzs = sys.argv[1]
    intd = b"UNIX" + bytes([0, 0, 0, 0]) + b"    " + b"/tmp/story.z3"
    valid_second_frame = bytes([0, 0x12, 0x34, 1, 0, 1, 0, 1, 0, 2, 0, 3])

    with tempfile.TemporaryDirectory(prefix="ckifzs-conformance.") as tmp:
        directory = Path(tmp)

        check(ckifzs, directory, "intd.qzl", fixture(IFHD, DUMMY_FRAME, chunk(b"IntD", intd)), 0,
              "Save file is valid.")
        check(ckifzs, directory, "unknown.qzl", fixture(IFHD, DUMMY_FRAME, chunk(b"Zzzz", b"abcd")), 0,
              "unknown chunk; skipped")
        check(ckifzs, directory, "trailing.qzl", fixture() + bytes(8), 0,
              "warning: spurious data (8 bytes)")

        check(ckifzs, directory, "ifhd-0.qzl", fixture(b""), 1, "IFhd chunk is too short")
        check(ckifzs, directory, "ifhd-12.qzl", fixture(IFHD[:12]), 1, "IFhd chunk is too short")
        check(ckifzs, directory, "ifhd-13.qzl", fixture(IFHD), 0, "Save file is valid.")
        check(ckifzs, directory, "ifhd-14.qzl", fixture(IFHD + b"x"), 0, "Save file is valid.")

        check(ckifzs, directory, "stks-empty.qzl", fixture(IFHD, b""), 0,
              "Save file is valid.")
        check(ckifzs, directory, "stks-ragged.qzl", fixture(IFHD, bytes([1]) * 7), 1,
              "at least 8 are required")
        check(ckifzs, directory, "stks-huge.qzl", fixture(IFHD, bytes([0, 0, 0, 0, 0, 0, 0xFF, 0xFF])),
              1, "value bytes")
        check(ckifzs, directory, "stks-second-ragged.qzl", fixture(IFHD, DUMMY_FRAME + bytes([1]) * 7),
              1, "Stks frame 2")
        check(ckifzs, directory, "stks-local-missing.qzl", fixture(IFHD, bytes([0, 0, 0, 1, 0, 0, 0, 0])),
              1, "requires 2 value bytes")
        check(ckifzs, directory, "stks-two-frames.qzl", fixture(IFHD, DUMMY_FRAME + valid_second_frame),
              0, "Save file is valid.")

        duplicate_ifhd = fixture(IFHD, DUMMY_FRAME, chunk(b"IFhd", b""))
        check(ckifzs, directory, "duplicate-ifhd.qzl", duplicate_ifhd, 0, "warning: later IFhd")
        duplicate_stks = fixture(IFHD, DUMMY_FRAME, chunk(b"Stks", bytes([1]) * 7))
        check(ckifzs, directory, "duplicate-stks.qzl", duplicate_stks, 0, "warning: later Stks")
        duplicate_memory = fixture(IFHD, DUMMY_FRAME, chunk(b"UMem", b""))
        check(ckifzs, directory, "duplicate-memory.qzl", duplicate_memory, 0, "warning: later memory")

    print("ckifzs conformance regression test passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
