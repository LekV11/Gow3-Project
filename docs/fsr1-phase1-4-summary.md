# FSR 1 Research: Phase 1–4 Summary

Branch: `fsr1-research`  
Base tag: `windows-v2.3.0` (`f80021b`)  
Completed: 2026-11-04

---

## 1. Commits in fsr1-research (newest first)

### `3cb3715c81121fedc6110f3d74fb9c9ae1c9ec04` — Phase 3 (Launcher)
**Message:** `fsr1(launcher): restore FSR1/RCAS/colorspace controls, warn when EASU cannot run`  
**Files changed:** `launcher/gow3_launcher_win.py` (+70 lines)  
**What changed and why:** Added a new `_build_fsr1_section()` method to `build_graphics()` that restores the FSR 1 (EASU) checkbox, RCAS checkbox, RCAS strength slider, and colorspace dropdown — all writing to `gow3.ini` — and adds a warning label that appears when `fsr1=1` but `render_resolution` is native or 1080p (EASU cannot upscale same-size input).

---

### `604e651d01e120fd218c127048f52743161e9bcb` — Fix: RCAS-only darkening
**Message:** `fsr1: fix RCAS-only darkening, share easu-runs flag`  
**Files changed:** `gpu/CMakeLists.txt`, `gpu/shadps4/video_core/renderer_vulkan/vk_presenter.cpp`, `gpu/shim/gow3_settings.cpp`, `tests/test_fsr1_colorspace.cpp`  
**What changed and why:** Introduced `easu_runs_this_frame` in `vk_presenter.cpp` to gate `colorspace_perceptual`; without this guard, the default RCAS-only path received `Sample[1]=1` (gamma decode) on pixels that were never gamma-encoded, causing visible darkening. Added `fsr1-colorspace-test` to verify the gating logic.

---

### `5017173eb439fd37b688623d09f3cfe3038da094` — Commit 3: Colorspace wrapper
**Message:** `fsr1: add GOW3_FSR1_COLORSPACE wrapper (perceptual/linear) around EASU+RCAS`  
**Files changed:** `gpu/shim/gow3_settings.h`, `gpu/shim/gow3_settings.cpp`, `gpu/shadps4/video_core/renderer_vulkan/host_passes/fsr_pass.h`, `gpu/shadps4/video_core/renderer_vulkan/host_passes/fsr_pass.cpp`, `gpu/shadps4/video_core/host_shaders/fsr.comp`, `tests/test_graphics_settings.cpp`  
**What changed and why:** Added `fsr1_colorspace_perceptual` setting (env `GOW3_FSR1_COLORSPACE`); when `perceptual` (default), `sqrt()` encodes EASU output to gamma 2.0 before RCAS and `c*c` decodes it after, matching AMD FSR 1 spec. When `linear`, the old no-conversion behavior is preserved.

---

### `b28f24bd9ab3cf79ad924f40a776086ab9d7e185` — Commit 2: Startup log
**Message:** `fsr1: startup log and env-var audit for rcas-strength and overlay label`  
**Files changed:** `gpu/shim/gow3_settings.cpp`, `gpu/shim/gow3_overlay.cpp`  
**What changed and why:** Added a `[FSR]` startup log line printing EASU/RCAS status, rcas_strength, render size, output size, and colorspace; fixed `GOW3_RCAS_STRENGTH` missing from the env-var table; updated the perf overlay label to show `FSR1`, `FSR1+RCAS`, or the existing upscaler name.

---

### `058d193f78e92e8d6687f7886c50c8160ee3d699` — Commit 1: Remove hardcoded-false guard
**Message:** `fsr1: remove hardcoded-false guard so fsr1 ini/env key is respected`  
**Files changed:** `gpu/shim/gow3_settings.cpp`, `tests/test_graphics_settings.cpp`  
**What changed and why:** Removed the line `v.fsr1 = false` that silently ignored any `fsr1=1` in `gow3.ini` or `GOW3_FSR1=1` env var; updated tests to assert the value is now read and respected.

---

### `f80021b0b6d9ef6b6c91624b0ed9ae7a3ecb3f51` — Baseline (tag: windows-v2.3.0)
**Message:** `Chore(Release): bumped version to 2.3.0 (#340)`  
**Files changed:** version files, release notes  
**What changed and why:** Official 2.3.0 release. FSR 1 was present in the source (RCAS was active) but the `fsr1` setting was always forced to `false` in `gow3_settings.cpp` and had no launcher control.

---

## 2. How to build from source

### Prerequisites

1. Install MSYS2:
   ```powershell
   winget install MSYS2.MSYS2
   ```
2. Open the **MSYS2 CLANG64** shell (Start → MSYS2 CLANG64).

3. Install the toolchain and all required libraries (from `packaging/windows/README.md` at tag `windows-v2.3.0`):
   ```bash
   pacman -S --needed git patch \
     mingw-w64-clang-x86_64-{toolchain,cmake,ninja,pkgconf,python} \
     mingw-w64-clang-x86_64-{vulkan-headers,vulkan-loader,vulkan-memory-allocator,glslang} \
     mingw-w64-clang-x86_64-{sdl3,fmt,boost,robin-map,xxhash,ffmpeg,zydis} \
     mingw-w64-clang-x86_64-{spirv-tools,spirv-headers,spirv-cross}
   ```
   (`magic_enum`, `miniz`, and `xbyak` are in the repo as submodules under `gpu/third_party/` and do not need a separate pacman install.)

### Clone and checkout

```bash
git clone --recursive https://github.com/FelipePrado0/SpartanPort.git C:\src\SpartanPort
cd /c/src/SpartanPort
git checkout fsr1-research
git submodule update --init --recursive
```

### Build

In the MSYS2 CLANG64 shell:

```bash
cd /c/src/SpartanPort
bash build.sh
```

### Output binary

`build.sh` produces:

| File | Description |
|---|---|
| `out/gow3-probe.exe` | Main game launcher shim (runs the GPU library) |
| `out/gow3-gpu-capabilities.exe` | GPU capability probe |
| `out/gpu/libgow3gpu.a` | GPU library (static, linked into the probe) |

The release package (exe + DLLs) is produced by `bash packaging/windows/package.sh` — see Section 4.

### Run tests

```bash
ninja -C out/gpu graphics-settings-test perf-overlay-test warmup-inbox-test fsr1-colorspace-test
./out/gpu/graphics-settings-test.exe
./out/gpu/perf-overlay-test.exe
./out/gpu/warmup-inbox-test.exe
./out/gpu/fsr1-colorspace-test.exe
```

All four must print `PASS`.

---

## 3. How to run the 5 test conditions (720p render → 1080p output)

Before running any condition, set `render_resolution=720p` in `gow3.ini` for conditions 2–5 (or use the launcher's Render resolution dropdown). For condition 1 (native), `render_resolution=native` is the default.

`run.py` is the Windows equivalent of `run.sh`; it reads `gow3.ini` via the path in `GOW3_CONFIG` (default: `gow3.ini` next to the launcher).

**Note:** `GOW3_EXPERIMENTAL_UPSCALER` is **not needed** for any FSR 1 condition. FSR 1 (EASU/RCAS) is a separate post-process pipeline. `GOW3_EXPERIMENTAL_UPSCALER` only gates temporal upscalers (FSR 3.1, FSR 4, TAA, DLSS). Do not set it.

---

### Condition 1 — Native (no upscaling, no RCAS)

Render and output are the same size; no post-processing.

**gow3.ini:**
```ini
render_resolution=native
fsr1=0
rcas=0
```

**Run (PowerShell):**
```powershell
python run.py
```

---

### Condition 2 — Bilinear (presenter scale, no FSR)

The game renders at 720p; the presenter bilinearly scales to the output window. FSR 1 is off so no spatial upscaling or sharpening runs.

**gow3.ini:**
```ini
render_resolution=720p
fsr1=0
rcas=0
```

**Run (PowerShell):**
```powershell
python run.py
```

---

### Condition 3 — EASU only (FSR 1 upscale, no sharpening)

EASU spatially upscales 720p → 1080p. RCAS is off.

**gow3.ini:**
```ini
render_resolution=720p
fsr1=1
rcas=0
```

**Run (PowerShell):**
```powershell
set GOW3_FSR1=1 && set GOW3_RCAS=0 && python run.py
```

Or via gow3.ini only (no env vars needed):
```powershell
python run.py
```

---

### Condition 4 — EASU + RCAS perceptual (full FSR 1 per AMD spec)

EASU upscales 720p → 1080p; RCAS sharpens the result. Gamma 2.0 conversion wraps EASU+RCAS as AMD recommends in the FSR 1 header (`ffx_fsr1.h`): EASU output is encoded to approximate gamma 2.0 before RCAS, then decoded back to linear after RCAS.

**gow3.ini:**
```ini
render_resolution=720p
fsr1=1
rcas=1
fsr1_colorspace=perceptual
```

**Run (PowerShell):**
```powershell
set GOW3_FSR1=1 && set GOW3_RCAS=1 && set GOW3_FSR1_COLORSPACE=perceptual && python run.py
```

---

### Condition 5 — EASU + RCAS linear (bypass gamma wrap)

Same pipeline as condition 4, but the gamma 2.0 conversions are skipped. This reproduces the pre-commit-3 behavior (pipeline as implemented before the colorspace wrapper was added).

**gow3.ini:**
```ini
render_resolution=720p
fsr1=1
rcas=1
fsr1_colorspace=linear
```

**Run (PowerShell):**
```powershell
set GOW3_FSR1=1 && set GOW3_RCAS=1 && set GOW3_FSR1_COLORSPACE=linear && python run.py
```

---

## 4. How to update the release package

> **Documentation only — do not run these steps without preparing a release.**

The packaging procedure is in `packaging/windows/README.md` and `packaging/windows/package.sh`.

### Build artifacts to copy

Running `bash packaging/windows/package.sh` from the MSYS2 CLANG64 shell produces:

```
dist/gow3-windows/
  God of War III.exe          ← PyInstaller-frozen launcher (includes run.py, scripts/, launcher/)
  Play God of War III.exe     ← thin native wrapper (packaging/windows/play.c)
  libgow3gpu.dll (or .a)      ← GPU library (linked into gow3-probe.exe)
  gow3-probe.exe              ← game process
  gow3-gpu-capabilities.exe   ← GPU capability probe
  <MSYS2 runtime DLLs>        ← copied by package.sh from C:\msys64\clang64\bin
```

`dist/gow3-windows.zip` is the distributable archive.

### Signing / packaging steps

`package.sh` uses PyInstaller to freeze `launcher/gow3_launcher_win.py` (and all `launcher/` and `scripts/` Python files it imports) into `God of War III.exe` using a private venv at `out/pyenv`. No code-signing step is in the script; signing would be a manual step added on top of `package.sh` output.

### What changed in this branch that affects the package

| Component | Change |
|---|---|
| `launcher/gow3_launcher_win.py` | Phase 3 (commit `3cb3715`): adds FSR 1 / RCAS / colorspace UI controls that write to `gow3.ini`. PyInstaller bundles this automatically — no changes to `package.sh` are needed. |
| `gpu/shim/gow3_settings.cpp` | Commits 1–fix: removes hardcoded-false guard, adds startup log, adds `GOW3_FSR1_COLORSPACE`, updates log annotation. The GPU library is rebuilt and re-linked into `gow3-probe.exe`. |
| `gpu/shadps4/video_core/renderer_vulkan/vk_presenter.cpp` | Commit fix: `easu_runs_this_frame` guard preventing RCAS-only darkening. |
| `gpu/shadps4/video_core/host_shaders/fsr.comp` | Commit 3: gamma 2.0 `sqrt`/`c*c` branches controlled by `Sample[1]`. The compiled SPIR-V is embedded at build time. |

### Packaging procedure summary

```bash
# In MSYS2 CLANG64 shell, after build.sh succeeds:
bash packaging/windows/package.sh
# Output: dist/gow3-windows/ and dist/gow3-windows.zip
```

---

## 5. Known issues and limitations

- **GPU/pixel comparison tests not yet implemented.** Only CPU unit tests exist (`fsr1-colorspace-test`, `graphics-settings-test`). A full validation of the gamma 2.0 colorspace path requires a frame capture comparison (e.g., `GOW3_FRAME_DUMP_TRIGGER`) and a numerical pixel diff tool; this was not implemented in this branch.

- **Startup log prints render size `0x0` before the first frame.** `[FSR] ... render=0x0 output=0x0 ...` is expected. `image_width`/`image_height` are populated from `Presenter::PrepareFrame()`, which runs on the first frame; the log runs at `Load()` time before any frame is prepared.

- **Phase 3 in-game overlay (ImageTab in `gow3_overlay.cpp`) was explicitly decided NOT to implement.** The colorspace and FSR 1 status are visible in the launcher only. The perf overlay already shows `FSR1` or `FSR1+RCAS` as the upscaler label (commit 2, `b28f24b`), but no dedicated ImageTab for FSR 1 settings was added.

- **`GOW3_EXPERIMENTAL_UPSCALER` must NOT be set for FSR 1 conditions.** Setting it to `1` activates the temporal upscaler path (FSR 3.1, FSR 4, TAA, DLSS), which is a different pipeline. The launcher hard-codes `GOW3_EXPERIMENTAL_UPSCALER=0` (commit `22bce07`, pre-branch). FSR 1 / EASU / RCAS work independently of this variable.

- **`fsr1_colorspace` in the launcher only has effect when `fsr1=1`.** The `easu_runs_this_frame` guard in `vk_presenter.cpp` (commit `604e651`) ensures that even if `fsr1_colorspace=perceptual` is written to `gow3.ini`, `colorspace_perceptual` at runtime is `false` unless EASU actually dispatches. The launcher disables the colorspace dropdown when `fsr1` is off, reinforcing this.

- **`rcas_strength` already has a `GOW3_RCAS_STRENGTH` env override** (added in commit `b28f24b`). Do not add a second override or duplicate the env-var table entry.

- **Warning condition in the launcher uses a string comparison against `render_resolution`.** The known non-upscaling values are `'native'`, `'1080p'`, and `''` (empty/default). Any explicitly lower value (e.g., `720p`, `1440p` used in a downscale) is treated as "lower than output" and the warning is hidden. If a user sets `output_res` to something other than `1920x1080`, the warning logic does not account for it — it only checks `render_resolution`, not the relationship between `render_resolution` and `output_res`.
