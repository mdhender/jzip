#!/bin/sh

set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
story="$root/testdata/stories/zork1-r119-880429.z3"
fixture="$root/testdata/frotz/zork1-r119-kitchen.qzl"
dfrotz=${DFROTZ:-/opt/homebrew/bin/dfrotz}
jzip=${JZIP:-$root/jzip}
ckifzs=${CKIFZS:-$root/ckifzs}
tmp=$(mktemp -d "${TMPDIR:-/tmp}/jzip-interop.XXXXXX")
trap 'rm -rf "$tmp"' EXIT INT TERM

if [ ! -x "$dfrotz" ]; then
    echo "Frotz executable not found: $dfrotz" >&2
    exit 1
fi

if [ ! -x "$jzip" ]; then
    echo "Jzip executable not found: $jzip" >&2
    exit 1
fi

if [ ! -x "$ckifzs" ]; then
    echo "ckifzs executable not found: $ckifzs" >&2
    exit 1
fi

require_text()
{
    file=$1
    text=$2
    if ! grep -Fq "$text" "$file"; then
        echo "Expected text not found in $file: $text" >&2
        exit 1
    fi
}

# Drive Jzip as a line-oriented stdin/stdout filter.
(
    cd "$tmp"
    printf 'north\neast\nopen window\nenter\nsave\njzip.qzl\nquit\ny\n' |
        "$jzip" "$story" >jzip-save.log
)
require_text "$tmp/jzip-save.log" "Kitchen"
require_text "$tmp/jzip-save.log" "Ok."
"$ckifzs" "$tmp/jzip.qzl" >"$tmp/ckifzs-jzip.log"

# Restore the committed Frotz fixture in Jzip.
printf 'restore\n%s\nlook\nquit\ny\n' "$fixture" |
    "$jzip" "$story" >"$tmp/jzip-restore-frotz.log"
require_text "$tmp/jzip-restore-frotz.log" "Ok."
require_text "$tmp/jzip-restore-frotz.log" "You are in the kitchen of the white house."

# Establish the same movement transcript with Frotz and make a temporary save
# independently of the committed fixture.
(
    cd "$tmp"
    printf 'north\neast\nopen window\nenter\nsave\nfrotz.qzl\nquit\ny\n' |
        "$dfrotz" -p -m -w 80 "$story" >frotz-save.log
    printf 'restore\nfrotz.qzl\nlook\nquit\ny\n' |
        "$dfrotz" -p -m -w 80 "$story" >frotz-restore.log
)
require_text "$tmp/frotz-save.log" "Kitchen"
require_text "$tmp/frotz-save.log" "Score: 10       Moves: 4"
require_text "$tmp/frotz-restore.log" "You are in the kitchen of the white house."
"$ckifzs" "$tmp/frotz.qzl" >"$tmp/ckifzs-frotz.log"

# Restore Jzip's temporary save in Frotz.
(
    cd "$tmp"
    printf 'restore\njzip.qzl\nlook\nquit\ny\n' |
        "$dfrotz" -p -m -w 80 "$story" >frotz-restore-jzip.log
)
require_text "$tmp/frotz-restore-jzip.log" "You are in the kitchen of the white house."

echo "Jzip/Frotz Quetzal interoperability test passed."
