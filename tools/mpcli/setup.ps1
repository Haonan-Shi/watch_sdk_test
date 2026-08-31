# Copyright (c) 2026, Realtek Semiconductor Corporation
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#
# Install the mpcli flash tool so `mpcli` is PERMANENTLY on PATH (Windows).
#
# mpcli.exe resolves its fw/ and config/ directories from its OWN location,
# so a PATH entry works from any directory.
#
# What this does (idempotent, safe to re-run):
#   1. adds the mpcli dir to the current PowerShell session
#   2. permanently adds it to the user-scoped PATH (no admin rights needed),
#      so `mpcli` works in every new terminal
#
# Usage (from this folder):
#   .\setup.ps1
#
# If script execution is blocked, run once:
#   Set-ExecutionPolicy -Scope CurrentUser RemoteSigned

# --- resolve the platform directory next to this script ----------------------
$mpcliDir = Join-Path $PSScriptRoot 'Windows'
$mpcliBin = Join-Path $mpcliDir 'mpcli.exe'
if (-not (Test-Path $mpcliBin)) {
    Write-Warning "mpcli.exe not found at $mpcliBin"
}

# --- 1. current session ------------------------------------------------------
if (($env:PATH -split ';') -notcontains $mpcliDir) {
    $env:PATH = "$mpcliDir;$env:PATH"
}

# --- 2. permanent user-scoped PATH -------------------------------------------
$userPath  = [Environment]::GetEnvironmentVariable('PATH', 'User')
$userPaths = if ($userPath) { $userPath -split ';' } else { @() }
if ($userPaths -notcontains $mpcliDir) {
    $newPath = if ($userPath) { "$mpcliDir;$userPath" } else { $mpcliDir }
    [Environment]::SetEnvironmentVariable('PATH', $newPath, 'User')
    Write-Host "PATH: permanently added to USER PATH."
} else {
    Write-Host "PATH: already present in USER PATH."
}

Write-Host "Done. 'mpcli' is usable in this session; open a new terminal for other shells."
