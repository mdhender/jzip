# Jzip

Jzip is a small Z-Machine interpreter forked from
[Jzip 2.1](https://jzip.sourceforge.net/), a circa-2000 interpreter also
documented by [IFWiki](https://www.ifwiki.org/Jzip). This fork targets native
macOS on Apple Silicon and is intentionally optimized for scripted
compatibility testing rather than interactive play.

The interpreter reads commands from standard input and writes plain text to
standard output. It has no terminal control sequences, status window,
pagination, or interactive line editor, making it suitable for test harnesses
that compare Z-Machine and Quetzal behavior with modern interpreters such as
Frotz.

## Status

- Builds natively on macOS/arm64 with Apple Clang; no 32-bit target is needed.
- Supports the Jzip 2.1 story formats: Z-Machine versions 1–5 and 8.
- Reads gzip-compressed stories through zlib.
- Reads Quetzal saves containing either `CMem` or `UMem` dynamic memory.
- Writes `CMem` by default and can write `UMem` with `-u`.
- Includes `ckifzs` for structural validation of Quetzal/IFZS files.
- Tests Zork I, II, and III and cross-checks Quetzal saves with Frotz.

Versions 6 and 7 are accepted by parts of the legacy interpreter but are not
claimed as supported by this fork because they have not been implemented and
tested as complete story formats.

This is a deliberate fork. Historical platform implementations and build
definitions are removed when they are no longer useful; Git history preserves
them.

## Requirements

- macOS on Apple Silicon
- Apple Clang
- CMake 3.20 or newer
- zlib from the macOS SDK
- `/opt/homebrew/bin/dfrotz` for the Frotz comparison tests

The Frotz location can be overridden when configuring CMake:

```sh
cmake -S . -B cmake-build-debug \
    -DDFROTZ_EXECUTABLE=/path/to/dfrotz
```

## Build

```sh
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug
```

The build produces:

- `cmake-build-debug/jzip` — the Z-Machine interpreter
- `cmake-build-debug/ckifzs` — the Quetzal structure checker

The root project can also be opened directly in JetBrains CLion.

## Run Jzip

Pipe one game response per line:

```sh
printf 'north\neast\nopen window\nenter\nquit\ny\n' |
    cmake-build-debug/jzip testdata/stories/zork1-r119-880429.z3
```

Shell redirection is the supported script and transcript interface:

```sh
cmake-build-debug/jzip story.z3 <commands.txt >transcript.log
```

See the [`jzip` command reference](doc/jzip.md) for all options and save/restore
examples.

## Quetzal memory formats

Jzip writes compressed `CMem` chunks by default:

```sh
cmake-build-debug/jzip story.z3
```

Pass `-u` to write uncompressed `UMem` chunks instead:

```sh
cmake-build-debug/jzip -u story.z3
```

The option affects only saves created during that run. Restoring supports both
formats regardless of the option. Inspect a save with:

```sh
cmake-build-debug/ckifzs save.qzl
```

See the [`ckifzs` command reference](doc/ckifzs.md) for details.

## Test

Run the normal test suite:

```sh
ctest --test-dir cmake-build-debug --output-on-failure
```

The suite contains Zork I/II/III smoke tests, a deterministic standard-I/O
transcript test, and bidirectional Jzip/Frotz Quetzal interoperability tests.
It verifies Jzip saves using both `CMem` and `UMem`.

Run the same suite with AddressSanitizer and UndefinedBehaviorSanitizer:

```sh
cmake -S . -B cmake-build-sanitize \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_SANITIZERS=ON
cmake --build cmake-build-sanitize
ctest --test-dir cmake-build-sanitize --output-on-failure
```

The committed story fixtures are redistributable builds of Zork I, II, and III.
Their provenance and test layout are documented in
[`testdata/README.md`](testdata/README.md).

## License

Jzip is distributed under the BSD-style terms in [`LICENSE`](LICENSE). The
original source is copyright John D. Holder; changes in this fork are copyright
Michael Henderson.

The story files under `testdata/stories` are separate works distributed under
the MIT License. Each is governed by the adjacent `LICENSE.<story>.txt` file.

## Authors

- John D. Holder — original Jzip author
- Michael Henderson — macOS fork
- [Ampcode](https://ampcode.com/) — porting and development assistance
