# Project Guide

## Purpose

This repository contains Jzip 2.1, a circa-2000 Z-Machine interpreter sourced from <https://www.ifwiki.org/Jzip>. The immediate goal is to port it just far enough to compile and run natively on a current Apple Silicon Mac, then use its command-line interpreter and Quetzal support for compatibility testing against a modern Frotz installation.

Keep the initial port conservative: preserve interpreter behavior and file-format semantics while establishing a trustworthy baseline. This is a deliberate macOS fork, not an effort to preserve every historical target. Remove obsolete platform files and code once their irrelevance to the macOS interpreter and Quetzal tooling is understood; Git history is the archive.

## Platform Constraints

- Target native macOS/arm64 with the system Clang toolchain.
- The available compiler cannot produce 32-bit binaries. Never solve a portability problem by adding `-m32`, installing a 32-bit toolchain, or depending on an i386 runtime.
- Make data widths explicit where the old code assumed the ILP32 model. In particular, Z-Machine words are 16-bit and Quetzal chunk IDs, lengths, and other on-disk 32-bit values must remain exactly 32-bit even though `long` and `unsigned long` are 64-bit on macOS.
- Prefer `<stdint.h>` fixed-width types at file-format and virtual-machine boundaries. Do not globally replace every `long`; distinguish host file offsets and sizes from values whose width is specified by the Z-Machine or Quetzal format.
- Preserve big-endian byte-by-byte serialization. Do not write C structs directly to story or save files.
- Avoid assumptions about host endianness, alignment, struct padding, pointer size, or the representation of `long`.

## Relevant Code

- `Makefile`: supported native macOS command-line build.
- `CMakeLists.txt`: equivalent CMake build used by CLion, including the interoperability test.
- `unixio.mak`: legacy Unix build retained only until the macOS build and required source set are established.
- `unixio.c`: terminal/termcap implementation used by the Unix build.
- `ztypes.h`: shared configuration, VM types, constants, globals, and function declarations. `USE_QUETZAL` is enabled here.
- `fileio.c`: story-file access plus save/restore dispatch. With `USE_ZLIB`, modern zlib defines `gzFile` as an opaque pointer typedef; the legacy `gzFile *` declarations are therefore one pointer level too deep.
- `quetzal.c`: Quetzal IFZS serialization and restoration. Audit its `unsigned long` format values carefully on LP64.
- `ckifzs.c`: standalone Quetzal structure checker and a useful independent validation tool.
- `doc/ckifzs.md`: command reference for the checker.
- `jzip.c`, `interpre.c`, and the opcode modules: interpreter startup and VM execution.
- `doc/Building.txt`, `doc/Jzip.txt`, and `doc/jzip.6`: original build and command-line documentation. Treat these as historical references; verify details against current code.

Jzip supports Z-code versions 1-5 and 8. Do not imply support for versions 6 or 7 without implementing and testing it.

## Build Workflow

Use the native root makefile:

```sh
make
```

The supported products are `jzip` and `ckifzs`. `jzexe` creates historical self-contained DOS/Linux executables and is not part of the macOS port unless a concrete test requires it. The macOS SDK supplies the current link dependencies, zlib and termcap (`-lz -ltermcap`).

Clean generated files with:

```sh
make clean
```

Do not commit binaries, object files, generated save files, downloaded story files, or copyrighted game data.

## Implementation Guidelines

- Make the smallest change that fixes a demonstrated macOS/LP64 incompatibility.
- Retain the existing C style in touched code. Avoid bulk formatting and broad warning cleanup.
- Keep `USE_QUETZAL` enabled. Quetzal compatibility is the reason for this port.
- Keep gzip story support unless a specific, documented macOS blocker requires temporarily isolating it.
- Fix declarations and ownership at their source rather than adding casts that silence incompatible-pointer warnings.
- Do not truncate host file positions merely to recreate 32-bit behavior. Convert to a fixed-width format value only after checking its valid range.
- Treat compiler warnings about pointer types, integer conversion, format strings, signedness, and implicit declarations as possible LP64 bugs; investigate rather than suppressing them globally.
- Keep platform-specific changes in the Unix/macOS path. Do not spend effort preserving DOS, OS/2, Atari, Borland, or Quick C builds. Before deleting legacy code, confirm that the native interpreter does not reference it; then prefer deletion over maintaining dead compatibility branches.
- Do not change story/save compatibility to match a single fixture. The implementation must follow the Z-Machine and Quetzal formats generally.

## Verification

Scale verification with each change, but a porting change is not complete merely because it compiles.

1. Start from a clean tree and build with `make`.
2. Run `./jzip -v` and `./jzip -h` as non-interactive smoke checks.
3. Run `./ckifzs save-file` on each Quetzal fixture and require a successful structural check.
4. Launch Jzip with the story file that matches the save file:

   ```sh
   ./jzip path/to/story.z5
   ```

   Exercise restore through the story's normal `restore` command and confirm that play resumes at the expected state. Save-file identity includes the story release, serial number, and checksum, so an arbitrary story file is not a valid substitute.
5. Test the same story/save pair with command-line Frotz at `/opt/homebrew/bin/dfrotz` and compare observable restored state and subsequent commands. Record the exact Jzip and Frotz invocations and versions in test notes.
6. When save support changes, test both directions when possible: Jzip must read a Frotz-created Quetzal file, and Frotz plus `ckifzs` must read a Jzip-created file.

Jzip uses terminal control and interactive input, so tests that exercise gameplay should run in a real terminal or a PTY rather than assuming ordinary stdin/stdout redirection faithfully represents behavior. Keep external story files and Quetzal fixtures outside version control unless they are explicitly known to be redistributable. The committed Zork fixtures are separately distributed under the MIT licenses adjacent to the story files.

If a required fixture, matching story file, or Frotz executable is unavailable, complete the build and smoke checks that are possible and state exactly which compatibility checks remain unverified.
