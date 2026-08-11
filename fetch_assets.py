#!/usr/bin/env python3
"""fetch_assets.py — download this release's binary archives and restore them in place.

The large binaries (bin/, tools/) are not stored in git; they are published as
archives in the matching GitHub Release. This script reads ``assets.yaml`` (which
sits next to it at the repository root), downloads each archive from its URL,
verifies the sha256, and extracts it so ``bin/`` and ``tools/`` are restored.

The archives always match the checked-out version: ``assets.yaml`` is versioned in
git, so checking out a tag/commit gives you that version's URLs automatically.

Usage (run from anywhere — paths resolve relative to this file):
    python fetch_assets.py                 # fetch + verify + extract what's missing/changed
    python fetch_assets.py --force         # re-download & re-extract even if already present
    python fetch_assets.py --keep-archives # keep the downloaded .zip files (default: delete)
    python fetch_assets.py --dir <path>    # extract root (default: this file's directory)

Public repositories need no credentials. Requires PyYAML (a Zephyr west environment
already ships it); install with ``pip install pyyaml`` otherwise.
"""
from __future__ import annotations

import argparse
import hashlib
import sys
import urllib.request
import zipfile
from pathlib import Path

try:
    import yaml
except ImportError:
    sys.exit("需要 PyYAML:请先 `pip install pyyaml`(Zephyr west 环境通常已自带)。")

HERE = Path(__file__).resolve().parent
MANIFEST = HERE / "assets.yaml"
MARKER = ".asset_sha256"          # 每个目录下的落地标记,记录已解压 archive 的 sha256


def sha256_file(path: Path, chunk: int = 1 << 20) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(chunk), b""):
            h.update(block)
    return h.hexdigest()


def download(url: str, dest: Path) -> None:
    """流式下载,带简单进度条(不把整包读进内存)。"""
    dest.parent.mkdir(parents=True, exist_ok=True)
    with urllib.request.urlopen(url) as resp:
        total = int(resp.headers.get("Content-Length", 0))
        done = 0
        with open(dest, "wb") as f:
            while True:
                block = resp.read(1 << 20)
                if not block:
                    break
                f.write(block)
                done += len(block)
                if total:
                    pct = done * 100 // total
                    print(f"\r    下载 {dest.name}: {pct:3d}%  "
                          f"({done / (1 << 20):.1f}/{total / (1 << 20):.1f} MB)", end="")
                else:
                    print(f"\r    下载 {dest.name}: {done / (1 << 20):.1f} MB", end="")
    print()


def safe_extract(zip_path: Path, root: Path) -> None:
    """解压前校验成员路径不越界(防 zip-slip),再全部解压到 root。"""
    root = root.resolve()
    with zipfile.ZipFile(zip_path) as zf:
        for name in zf.namelist():
            dest = (root / name).resolve()
            if root not in dest.parents and dest != root:
                raise SystemExit(f"压缩包含越界路径,拒绝解压: {name}")
        zf.extractall(root)


def process(entry: dict, root: Path, force: bool, keep: bool) -> None:
    adir = entry["dir"]
    url = entry["url"]
    want = entry["sha256"]
    name = entry.get("archive", f"{adir}.zip")

    marker = root / adir / MARKER
    if not force and marker.is_file() and marker.read_text(encoding="utf-8").strip() == want:
        print(f"  [{adir}] 已就位 (sha256 匹配),跳过")
        return

    archive = root / name
    print(f"  [{adir}] 下载并校验 {name} ...")
    download(url, archive)

    got = sha256_file(archive)
    if got != want:
        archive.unlink(missing_ok=True)
        raise SystemExit(f"  [{adir}] sha256 不匹配!\n    期望: {want}\n    实际: {got}\n"
                         f"    已删除损坏文件,请重试。")
    print(f"  [{adir}] sha256 校验通过")

    safe_extract(archive, root)
    (root / adir).mkdir(parents=True, exist_ok=True)
    marker.write_text(want + "\n", encoding="utf-8")
    print(f"  [{adir}] 已解压到 {root / adir}")

    if not keep:
        archive.unlink(missing_ok=True)


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="下载并还原本 release 的二进制压缩包")
    ap.add_argument("--dir", help="解压根目录(默认:本脚本所在目录)")
    ap.add_argument("--force", action="store_true", help="即使已就位也重新下载/解压")
    ap.add_argument("--keep-archives", action="store_true", help="解压后保留 .zip(默认删除)")
    args = ap.parse_args(argv)

    if not MANIFEST.is_file():
        raise SystemExit(f"找不到清单: {MANIFEST}(应与本脚本同目录)")

    with open(MANIFEST, "r", encoding="utf-8") as f:
        manifest = yaml.safe_load(f)

    root = Path(args.dir).resolve() if args.dir else HERE
    archives = manifest.get("archives", []) or []
    version = manifest.get("version", "?")
    print(f"版本: {version}   解压根: {root}   共 {len(archives)} 个压缩包")

    for entry in archives:
        process(entry, root, args.force, args.keep_archives)

    print("完成:所有压缩包已就位。")
    return 0


if __name__ == "__main__":
    sys.exit(main())
