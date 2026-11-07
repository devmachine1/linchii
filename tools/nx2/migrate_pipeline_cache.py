#!/usr/bin/env python3
"""Carry a title's Vulkan pipeline cache over a VulkanPipelineCacheVersion bump.

The cache stores guest shader environments, not SPIR-V, so entries stay valid when only the
codegen changed: the emulator rebuilds them at boot. This merges <src>/vulkan.fork-v<from>.bin
into <dst>/vulkan.fork-v<to>.bin (header rewritten, existing entries kept first) and unions the
vulkan_used lists. Close the emulator first. Originals are backed up under <dst>/backup-<time>.

  python -I tools/nx2/migrate_pipeline_cache.py --src <old shader/<title>> --dst <new shader/<title>>
  python -I tools/nx2/migrate_pipeline_cache.py --selftest
"""
import argparse
import shutil
import struct
import sys
import tempfile
import time
from pathlib import Path

MAGIC = b"yuzucach"
USED_MAGIC = 0x31505545  # "EUP1"
HEADER = len(MAGIC) + 4


def read_cache(path, version):
    data = path.read_bytes()
    if data[: len(MAGIC)] != MAGIC or struct.unpack_from("<I", data, len(MAGIC))[0] != version:
        raise SystemExit(f"{path}: not a v{version} pipeline cache")
    return data[HEADER:]


def read_used(path):
    data = path.read_bytes()
    magic, count = struct.unpack_from("<II", data)
    if magic != USED_MAGIC or len(data) < 8 + count * 9:
        raise SystemExit(f"{path}: bad used-pipelines list")
    return {h: a for h, a in (struct.unpack_from("<QB", data, 8 + i * 9) for i in range(count))}


def write_used(path, used):
    path.write_bytes(
        struct.pack("<II", USED_MAGIC, len(used))
        + b"".join(struct.pack("<QB", h, a) for h, a in used.items())
    )


def migrate(src, dst, old, new):
    src_cache, dst_cache = src / f"vulkan.fork-v{old}.bin", dst / f"vulkan.fork-v{new}.bin"
    src_used, dst_used = src / f"vulkan_used.fork-v{old}.bin", dst / f"vulkan_used.fork-v{new}.bin"
    if not src_cache.exists():
        raise SystemExit(f"{src_cache} not found")

    body_old = read_cache(src_cache, old)
    body_new = read_cache(dst_cache, new) if dst_cache.exists() else b""
    used = read_used(src_used) if src_used.exists() else {}
    if dst_used.exists():
        for h, a in read_used(dst_used).items():
            used[h] = min(a, used.get(h, a))

    backup = dst / time.strftime("backup-%Y%m%d-%H%M%S")
    backup.mkdir(parents=True)
    for f in (dst_cache, dst_used):
        if f.exists():
            shutil.copy2(f, backup / f.name)

    # Existing v<new> entries first: a bad tail in the old file then cannot swallow them.
    dst_cache.write_bytes(MAGIC + struct.pack("<I", new) + body_new + body_old)
    if used:
        write_used(dst_used, used)
    print(f"{dst_cache.name}: {len(body_new)} + {len(body_old)} body bytes; "
          f"{len(used)} used pipelines; backup in {backup}")


def selftest():
    with tempfile.TemporaryDirectory() as tmp:
        src, dst = Path(tmp, "src"), Path(tmp, "dst")
        src.mkdir(), dst.mkdir()
        (src / "vulkan.fork-v1.bin").write_bytes(MAGIC + struct.pack("<I", 1) + b"OLD")
        (dst / "vulkan.fork-v2.bin").write_bytes(MAGIC + struct.pack("<I", 2) + b"NEW")
        write_used(src / "vulkan_used.fork-v1.bin", {1: 5, 2: 3})
        write_used(dst / "vulkan_used.fork-v2.bin", {2: 0, 3: 1})
        migrate(src, dst, 1, 2)
        assert read_cache(dst / "vulkan.fork-v2.bin", 2) == b"NEWOLD"
        assert read_used(dst / "vulkan_used.fork-v2.bin") == {1: 5, 2: 0, 3: 1}
        assert len(list(dst.glob("backup-*/vulkan.fork-v2.bin"))) == 1
    print("selftest ok")


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    p.add_argument("--src", type=Path)
    p.add_argument("--dst", type=Path)
    p.add_argument("--from-version", type=int, default=53)
    p.add_argument("--to-version", type=int, default=54)
    p.add_argument("--selftest", action="store_true")
    a = p.parse_args()
    if a.selftest:
        selftest()
    elif a.src and a.dst:
        migrate(a.src, a.dst, a.from_version, a.to_version)
    else:
        p.error("--src and --dst are required")
