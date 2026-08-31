#!/usr/bin/env bash
#
# Copyright (c) 2026, Realtek Semiconductor Corporation
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#
# Install the mpcli flash tool so `mpcli` is PERMANENTLY available (Linux).
#
# mpcli resolves its fw/ and config/ directories from its OWN real path
# (via /proc/self/exe), so both a PATH entry and a symlink work from any
# working directory.
#
# What this does (idempotent, safe to re-run):
#   1. ensures the binary is executable
#   2. if a project .venv is found, symlinks mpcli into <venv>/bin so it is on
#      PATH whenever the venv is activated
#   3. permanently adds the mpcli dir to PATH via your shell rc (~/.bashrc or
#      ~/.zshrc) so `mpcli` works in every new shell
#   4. if this script was sourced, also updates the CURRENT shell right away
#
# Usage:
#   ./setup.sh          # install permanently (new shells + venv)
#   source ./setup.sh   # ...and also make it usable in THIS shell immediately

# --- resolve this script's own directory (bash or zsh) -----------------------
if [ -n "${BASH_SOURCE:-}" ]; then
    _self="${BASH_SOURCE[0]}"
elif [ -n "${ZSH_VERSION:-}" ]; then
    _self="${(%):-%N}"
else
    _self="$0"
fi
_root="$(cd "$(dirname "$_self")" >/dev/null 2>&1 && pwd)"

# --- Linux only --------------------------------------------------------------
if [ "$(uname -s)" != "Linux" ]; then
    echo "setup.sh: this repo only ships a Linux mpcli; use setup.ps1 on Windows." >&2
    return 1 2>/dev/null || exit 1
fi

_dir="$_root/Linux"
_bin="$_dir/mpcli"
if [ ! -f "$_bin" ]; then
    echo "setup.sh: mpcli binary not found at $_bin" >&2
    return 1 2>/dev/null || exit 1
fi

# --- 1. executable bit -------------------------------------------------------
[ -x "$_bin" ] || chmod +x "$_bin"

# --- 2. symlink into a project .venv (if any) --------------------------------
# prefer an already-active venv; otherwise walk up from the script for .venv
_venv=""
if [ -n "${VIRTUAL_ENV:-}" ] && [ -d "$VIRTUAL_ENV/bin" ]; then
    _venv="$VIRTUAL_ENV"
else
    _d="$_root"
    while [ "$_d" != "/" ]; do
        if [ -d "$_d/.venv/bin" ]; then _venv="$_d/.venv"; break; fi
        _d="$(dirname "$_d")"
    done
fi
if [ -n "$_venv" ]; then
    ln -sf "$_bin" "$_venv/bin/mpcli"
    echo "symlink: $_venv/bin/mpcli -> $_bin"
fi

# --- 3. permanent PATH via shell rc ------------------------------------------
case "$(basename "${SHELL:-bash}")" in
    zsh) _rc="$HOME/.zshrc" ;;
    *)   _rc="$HOME/.bashrc" ;;
esac
if [ -f "$_rc" ] && grep -qsF "$_dir" "$_rc"; then
    echo "PATH: already present in $_rc"
else
    printf '\n%s\n' "export PATH=\"$_dir:\$PATH\"  # mpcli flash tool" >> "$_rc"
    echo "PATH: added to $_rc"
fi

# --- 4. current shell (takes effect only when this script is sourced) --------
case ":$PATH:" in
    *":$_dir:"*) : ;;
    *) export PATH="$_dir:$PATH" ;;
esac

_sourced=0
if [ -n "${BASH_SOURCE:-}" ]; then
    [ "${BASH_SOURCE[0]}" != "$0" ] && _sourced=1
elif [ -n "${ZSH_VERSION:-}" ]; then
    case "$ZSH_EVAL_CONTEXT" in *:file) _sourced=1 ;; esac
fi

echo
if [ "$_sourced" -eq 1 ]; then
    echo "Done. 'mpcli' is installed permanently and usable in THIS shell now."
else
    echo "Done. 'mpcli' is installed permanently."
    echo "Open a new shell (or 'source $_rc') to use it now; inside the venv it works after 'activate'."
fi

unset _self _root _dir _bin _venv _d _rc _sourced
