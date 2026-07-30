# `ckifzs` command reference

## Name

`ckifzs` — check the structure of a Quetzal/IFZS save file

## Synopsis

```text
ckifzs FILE
```

## Description

`ckifzs` reads one Quetzal save file and reports its IFF `FORM IFZS` structure. It checks container and chunk lengths, required chunks, selected chunk ordering rules, and stack-frame boundaries.

The command prints the identifier and size of each chunk. For an `IFhd` chunk, it also prints the story release number, six-character serial number, checksum, and saved program counter.

`ckifzs` is a structural checker. It does not restore the game, compare the save with a story file, decompress `CMem`, or validate saved program counters and stack values against a story.

## Arguments

### `FILE`

Path to the Quetzal/IFZS save file to check. Exactly one path is required.

The command does not implement options, including `--help` or `--version`. A filename of `-` is not accepted as standard input.

## Checks

`ckifzs` performs the following checks:

- the file begins with an IFF `FORM` header and has form type `IFZS`;
- the declared `FORM` length is legal and contains complete chunk headers;
- chunk identifiers contain printable ASCII characters;
- each chunk fits within the declared `FORM` length, including odd-length padding;
- data following the declared `FORM` contents is reported as a warning;
- an `IFhd` header chunk of at least 13 bytes is present;
- a `Stks` stack chunk is present and consists of complete frames when nonempty;
- each stack frame contains the local variables and evaluation words declared by its header;
- at least one `CMem` or `UMem` memory chunk is present;
- `IFhd` precedes memory and stack chunks;
- later duplicates of singleton chunks are reported as warnings.

Unknown chunk identifiers are listed and skipped as required by the Quetzal standard.

## Recognized chunks

| Identifier | Description |
| --- | --- |
| `IFhd` | Quetzal story identity and saved program counter |
| `CMem` | Compressed dynamic memory |
| `UMem` | Uncompressed dynamic memory |
| `Stks` | Z-Machine call and evaluation stacks |
| `IntD` | Interpreter-dependent data |
| `ANNO` | Annotation text |
| `AUTH` | Author text |
| `NAME` | Content name |
| `(c) ` | Copyright text |
| four spaces | Filler chunk |

## Output

Normal output is written to standard output. Failure to open `FILE` and command usage are written to standard error.

A successful check ends with:

```text
Save file is valid.
```

Diagnostics for malformed structures begin with `***`. Non-fatal diagnostics include the word `warning`; unknown chunks are identified as skipped. Warnings do not change the exit status.

## Exit status

| Status | Meaning |
| --- | --- |
| `0` | The required chunks are present and no structural errors were reported. |
| `1` | The file was opened but is malformed, lacks a required chunk, or contains another conformance error. |
| `2` | Usage was invalid or the file could not be opened. |

## Examples

Check a Frotz-created save:

```sh
./ckifzs testdata/frotz/zork1-r119-kitchen.qzl
```

Use the exit status without displaying the report:

```sh
./ckifzs game.qzl >/dev/null
```

An exit status of zero means only that `ckifzs` accepted the save's structure. Restore the save with the matching story in Jzip and Frotz to test interpreter compatibility.
