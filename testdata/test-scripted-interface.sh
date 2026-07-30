#!/bin/sh

set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
story="$root/testdata/stories/zork1-r119-880429.z3"
script="$root/testdata/scripts/zork1-kitchen.txt"
jzip=${JZIP:-$root/jzip}
dfrotz=${DFROTZ:-/opt/homebrew/bin/dfrotz}
tmp=$(mktemp -d "${TMPDIR:-/tmp}/jzip-scripted.XXXXXX")
trap 'rm -rf "$tmp"' EXIT INT TERM

require_text()
{
    file=$1
    text=$2
    if ! grep -Fq "$text" "$file"; then
        echo "Expected text not found in $file: $text" >&2
        exit 1
    fi
}

reject_text()
{
    file=$1
    text=$2
    if grep -Fq "$text" "$file"; then
        echo "Unexpected text found in $file: $text" >&2
        exit 1
    fi
}

require_order()
{
    file=$1
    shift
    previous=0
    for text do
        line=$(grep -Fn "$text" "$file" | head -n 1 | cut -d: -f1)
        if [ -z "$line" ] || [ "$line" -le "$previous" ]; then
            echo "Text is missing or out of order in $file: $text" >&2
            exit 1
        fi
        previous=$line
    done
}

if [ ! -x "$jzip" ]; then
    echo "Jzip executable not found: $jzip" >&2
    exit 1
fi

if [ ! -x "$dfrotz" ]; then
    echo "Frotz executable not found: $dfrotz" >&2
    exit 1
fi

(
    unset TERM
    "$jzip" "$story" <"$script" >"$tmp/jzip.log" 2>"$tmp/jzip.err"
)

"$dfrotz" -p -m -w 80 "$story" <"$script" >"$tmp/frotz.log" 2>"$tmp/frotz.err"

require_text "$tmp/jzip.log" "ZORK I: The Great Underground Empire"
require_text "$tmp/jzip.log" "Release 119 / Serial number 880429"
require_order "$tmp/jzip.log" "West of House" "North of House" "Behind House" "Kitchen"
require_text "$tmp/jzip.log" "You are in the kitchen of the white house."
require_text "$tmp/jzip.log" "Your score is 10 (total of 350 points), in 4 moves."

require_order "$tmp/frotz.log" "West of House" "North of House" "Behind House" "Kitchen"
require_text "$tmp/frotz.log" "You are in the kitchen of the white house."
require_text "$tmp/frotz.log" "Your score is 10 (total of 350 points), in 4 moves."

reject_text "$tmp/jzip.log" "Score: 10"
reject_text "$tmp/jzip.log" "[MORE]"
escape=$(printf '\033')
reject_text "$tmp/jzip.log" "$escape"

if [ -s "$tmp/jzip.err" ]; then
    echo "Jzip wrote unexpected diagnostics:" >&2
    cat "$tmp/jzip.err" >&2
    exit 1
fi

# EOF is a clean request to stop, even when the game would otherwise ask for
# another command.
(
    unset TERM
    printf 'look\n' | "$jzip" "$story" >"$tmp/jzip-eof.log" 2>"$tmp/jzip-eof.err"
)
require_text "$tmp/jzip-eof.log" "West of House"
if [ -s "$tmp/jzip-eof.err" ]; then
    echo "Jzip reported an error at end of input" >&2
    cat "$tmp/jzip-eof.err" >&2
    exit 1
fi

echo "Jzip scripted interface test passed."
