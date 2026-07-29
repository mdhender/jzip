#!/bin/sh

set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
story="$root/testdata/stories/zork1-r119-880429.z3"
fixture="$root/testdata/frotz/zork1-r119-kitchen.qzl"
dfrotz=${DFROTZ:-/opt/homebrew/bin/dfrotz}
tmp=$(mktemp -d "${TMPDIR:-/tmp}/jzip-interop.XXXXXX")
trap 'rm -rf "$tmp"' EXIT INT TERM

if [ ! -x "$dfrotz" ]; then
    echo "Frotz executable not found: $dfrotz" >&2
    exit 1
fi

if ! command -v expect >/dev/null 2>&1; then
    echo "expect is required to drive Jzip through a pseudo-terminal" >&2
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

# Drive Jzip through a PTY because its terminal interface reads keystrokes and
# emits cursor-control sequences rather than behaving as a line filter.
ROOT=$root STORY=$story TMP=$tmp expect <<'EOF'
set timeout 15
log_user 0
expect_before timeout { exit 1 }
cd "$env(TMP)"
spawn env TERM=xterm "$env(ROOT)/jzip" "$env(STORY)"
expect ">"
send "north\r"
expect ">"
send "east\r"
expect ">"
send "open window\r"
expect ">"
send "enter\r"
expect "Kitchen"
expect ">"
send "save\r"
expect "Enter a file name."
expect ": "
send "jzip.qzl\r"
expect "Ok."
expect ">"
send "quit\r"
expect "Do you wish to leave the game?"
send "y\r"
expect "Hit any key to exit."
send "x"
expect eof
EOF
"$root/ckifzs" "$tmp/jzip.qzl" >"$tmp/ckifzs-jzip.log"

# Restore the committed Frotz fixture in Jzip.
ROOT=$root STORY=$story TMP=$tmp FIXTURE=$fixture expect <<'EOF'
set timeout 15
log_user 0
expect_before timeout { exit 1 }
cd "$env(TMP)"
spawn env TERM=xterm "$env(ROOT)/jzip" "$env(STORY)"
expect ">"
send "restore\r"
expect "Enter a file name."
expect ": "
send "$env(FIXTURE)\r"
expect "Ok."
expect ">"
send "look\r"
expect "You are in the kitchen of the white house."
expect ">"
send "quit\r"
expect "Do you wish to leave the game?"
send "y\r"
expect "Hit any key to exit."
send "x"
expect eof
EOF

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
"$root/ckifzs" "$tmp/frotz.qzl" >"$tmp/ckifzs-frotz.log"

# Restore Jzip's temporary save in Frotz.
(
    cd "$tmp"
    printf 'restore\njzip.qzl\nlook\nquit\ny\n' |
        "$dfrotz" -p -m -w 80 "$story" >frotz-restore-jzip.log
)
require_text "$tmp/frotz-restore-jzip.log" "You are in the kitchen of the white house."

echo "Jzip/Frotz Quetzal interoperability test passed."
