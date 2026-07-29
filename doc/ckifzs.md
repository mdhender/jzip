# `ckifzs` command reference

## Name

`ckifzs` — check the structure of a Quetzal/IFZS save file

## Synopsis

```text
ckifzs FILE
```

## Description

`ckifzs` reads one Quetzal save file and reports its IFF `FORM IFZS` structure. It checks container and chunk lengths, required chunks, selected chunk ordering rules, and duplicate known chunks.

The command prints the identifier and size of each chunk. For an `IFhd` chunk, it also prints the story release number, six-character serial number, checksum, and saved program counter.

`ckifzs` is a structural checker. It does not restore the game, compare the save with a story file, or fully validate the contents of compressed memory and stack frames.

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
- no data follows the declared `FORM` contents;
- an `IFhd` header chunk is present;
- a `Stks` stack chunk is present;
- at least one `CMem` or `UMem` memory chunk is present;
- `IFhd` precedes memory and stack chunks;
- known singleton chunks are not duplicated.

Unknown chunk identifiers are reported as errors by this implementation.

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

Diagnostics for malformed or unsupported structures begin with `***`. Some duplicate-chunk diagnostics include the word `warning`, but still cause a failure exit status.

## Exit status

| Status | Meaning |
| --- | --- |
| `0` | The required chunks are present and no structural errors were reported. |
| `1` | Usage was invalid, the file could not be opened, the file was malformed, a required chunk was absent, or another reported error occurred. |

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
