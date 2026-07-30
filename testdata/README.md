# Test Data

Files the tests read. Nothing here is part of the package.

```
testdata/
├── stories/   Z-machine story files
├── frotz/     save files made by Frotz
├── test-interop.sh
├── test-story-smoke.sh
└── README.md
```

## stories/

Story files are the input side of the tests.
The package never writes one.
They are here to provide a known baseline to read from.

Each name carries the release number and serial number from the story's own header:

```
zork1-r119-880429.z3
      ^^^^ ^^^^^^
      |    serial number
      release number
```

| File                   | Game     | Version | Release | Serial | Source                                                                                   |
|------------------------|----------|---------|---------|--------|------------------------------------------------------------------------------------------|
| `zork1-r119-880429.z3` | Zork I   | 3       | 119     | 880429 | [historicalsource/zork1](https://github.com/historicalsource/zork1), `COMPILED/zork1.z3` |
| `zork2-r63-860811.z3`  | Zork II  | 3       | 63      | 860811 | [historicalsource/zork2](https://github.com/historicalsource/zork2), `COMPILED/zork2.z3` |
| `zork3-r25-860811.z3`  | Zork III | 3       | 25      | 860811 | [historicalsource/zork3](https://github.com/historicalsource/zork3), `COMPILED/zork3.z3` |

Microsoft, Team Xbox, and Activision released the compiled Zork files under the MIT License.
Each story file has its own license file next to it, and that file is the one that applies.
Read it before you reuse a story file for anything.

## frotz/

Saves written by Frotz, an established interpreter.
They help assure us that this package works.

## Interoperability test

From the repository root, run:

```sh
cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug
ctest --test-dir cmake-build-debug --output-on-failure
```

The test uses the Zork I transcript documented in `frotz/README.md`. It pipes
commands to Jzip, compares the resulting Kitchen state with Frotz, validates
both interpreters' saves with `ckifzs`, and restores each interpreter's save
with the other interpreter. Generated files are kept in a temporary directory
and removed when the test exits.

CTest also pipes a quit transcript into each committed Zork story, checks its
title and release/serial banner, and verifies that Jzip exits normally. These
smoke tests run under both ordinary and sanitizer builds.
