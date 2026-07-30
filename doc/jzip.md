# `jzip` command reference

## Name

`jzip` — run a Z-Machine story as a plain standard-input/standard-output process

## Synopsis

```text
jzip [OPTIONS] STORY-FILE
```

## Description

This Jzip fork is a scriptable Z-Machine interpreter for story versions 1–5
and 8. It reads game commands from standard input and writes plain text to
standard output. It does not use terminal control sequences, a status window,
pagination, or interactive line editing. End of input stops the interpreter.

The primary purpose of this implementation is automated Z-Machine and Quetzal
compatibility testing on macOS. It can exchange Quetzal saves with other
interpreters, including Frotz.

Options must precede the story-file path. Exactly one story file is required.

## Options

### `-u`

Write uncompressed `UMem` dynamic-memory chunks in Quetzal save files created
during this run. Without this option, Jzip writes compressed `CMem` chunks.

This option changes only newly written saves. Jzip can restore both `CMem` and
`UMem` saves regardless of whether `-u` was specified.

### `-c COLUMNS`

Set the reported screen width in columns.

### `-l LINES`

Set the reported screen height in lines.

### `-r MARGIN`

Set the right text margin.

### `-t MARGIN`

Set the top text margin.

### `-k BYTES`

Set the command-history buffer size. The minimum is 1024 bytes and the maximum
is 16384 bytes. The plain standard-input implementation does not provide
interactive history editing, so this option is retained for compatibility.

### `-m`

Request monochrome mode.

### `-y`

Set the Z-Machine's Tandy compatibility bit.

### `-v`

Print version information and exit successfully. A story-file argument is not
required.

### `-z`

Print Jzip's license notice and exit successfully. A story-file argument is
not required.

### `-h`

Print command usage. The legacy argument parser returns a failure status after
displaying usage.

## Standard input and output

Supply one game response per line. Prompts and story output are written to
standard output. Runtime diagnostics are written to standard error. Input can
come from a pipe or redirected file, and output can be redirected normally:

```sh
./jzip story.z3 <commands.txt >transcript.log
```

Jzip does not provide separate input-script or output-log options; standard
shell redirection is the supported scripted interface.

## Quetzal saves

The story's `save` and `restore` commands prompt for a filename on standard
input. Relative filenames are resolved against Jzip's current working
directory.

By default, saves contain a `CMem` chunk. Use `-u` to create a `UMem` save:

```sh
printf 'north\neast\nopen window\nenter\nsave\nkitchen.qzl\nquit\ny\n' |
    ./jzip -u testdata/stories/zork1-r119-880429.z3 >save.log
```

Check the resulting structure:

```sh
./ckifzs kitchen.qzl
```

The report should identify `UMem (uncompressed memory)`. Structural validation
does not prove that the saved state is correct; restore the file with the
matching story and inspect the resulting game state:

```sh
printf 'restore\nkitchen.qzl\nlook\nquit\ny\n' |
    ./jzip testdata/stories/zork1-r119-880429.z3
```

Quetzal identifies its story by release number, serial number, and checksum. A
save must be restored with the matching story release.

## Environment

### `INFOCOM_PATH`

If `STORY-FILE` cannot be opened directly, Jzip searches the colon-separated
directories in `INFOCOM_PATH`.

## Exit status

Normal interpreter termination and informational `-v` or `-z` requests return
zero. Invalid command usage, `-h`, and fatal startup errors return nonzero.

## See also

- [`ckifzs` command reference](ckifzs.md)
- [`testdata` documentation](../testdata/README.md)
