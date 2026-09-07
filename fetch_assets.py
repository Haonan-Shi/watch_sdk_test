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
    sys.exit("PyYAML is required: run `pip install pyyaml` first "
             "(a Zephyr west environment usually already ships it).")

HERE = Path(__file__).resolve().parent
MANIFEST = HERE / "assets.yaml"
MARKER = ".asset_sha256"          # per-directory marker recording the sha256 of the extracted archive


def sha256_file(path: Path, chunk: int = 1 << 20) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for block in iter(lambda: f.read(chunk), b""):
            h.update(block)
    return h.hexdigest()


def download(url: str, dest: Path) -> None:
    """Stream the download with a simple progress bar (never load the whole file into memory)."""
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
                    print(f"\r    downloading {dest.name}: {pct:3d}%  "
                          f"({done / (1 << 20):.1f}/{total / (1 << 20):.1f} MB)", end="")
                else:
                    print(f"\r    downloading {dest.name}: {done / (1 << 20):.1f} MB", end="")
    print()


def safe_extract(zip_path: Path, root: Path) -> None:
    """Verify no member path escapes root (guard against zip-slip), then extract all into root."""
    root = root.resolve()
    with zipfile.ZipFile(zip_path) as zf:
        for name in zf.namelist():
            dest = (root / name).resolve()
            if root not in dest.parents and dest != root:
                raise SystemExit(f"archive contains an out-of-bounds path, refusing to extract: {name}")
        zf.extractall(root)


def process(entry: dict, root: Path, force: bool, keep: bool) -> None:
    adir = entry["dir"]
    url = entry["url"]
    want = entry["sha256"]
    name = entry.get("archive", f"{adir}.zip")

    marker = root / adir / MARKER
    if not force and marker.is_file() and marker.read_text(encoding="utf-8").strip() == want:
        print(f"  [{adir}] already in place (sha256 matches), skipping")
        return

    archive = root / name
    print(f"  [{adir}] downloading and verifying {name} ...")
    download(url, archive)

    got = sha256_file(archive)
    if got != want:
        archive.unlink(missing_ok=True)
        raise SystemExit(f"  [{adir}] sha256 mismatch!\n    expected: {want}\n    actual:   {got}\n"
                         f"    deleted the corrupted file, please retry.")
    print(f"  [{adir}] sha256 verified")

    safe_extract(archive, root)
    (root / adir).mkdir(parents=True, exist_ok=True)
    marker.write_text(want + "\n", encoding="utf-8")
    print(f"  [{adir}] extracted into {root / adir}")

    if not keep:
        archive.unlink(missing_ok=True)


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="download and restore this release's binary archives")
    ap.add_argument("--dir", help="extraction root (default: this script's directory)")
    ap.add_argument("--force", action="store_true", help="re-download / re-extract even if already in place")
    ap.add_argument("--keep-archives", action="store_true", help="keep the .zip files after extraction (default: delete)")
    args = ap.parse_args(argv)

    if not MANIFEST.is_file():
        raise SystemExit(f"manifest not found: {MANIFEST} (must sit next to this script)")

    with open(MANIFEST, "r", encoding="utf-8") as f:
        manifest = yaml.safe_load(f)

    root = Path(args.dir).resolve() if args.dir else HERE
    archives = manifest.get("archives", []) or []
    version = manifest.get("version", "?")
    print(f"version: {version}   extraction root: {root}   {len(archives)} archive(s)")

    for entry in archives:
        process(entry, root, args.force, args.keep_archives)

    print("done: all archives are in place.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
