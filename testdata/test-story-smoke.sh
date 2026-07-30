#!/bin/sh

set -eu

if [ "$#" -ne 4 ]; then
    echo "usage: $0 JZIP STORY TITLE RELEASE" >&2
    exit 1
fi

jzip=$1
story=$2
title=$3
release=$4

if [ ! -x "$jzip" ]; then
    echo "Jzip executable not found: $jzip" >&2
    exit 1
fi

if [ ! -f "$story" ]; then
    echo "Story file not found: $story" >&2
    exit 1
fi

if ! command -v expect >/dev/null 2>&1; then
    echo "expect is required to drive Jzip through a pseudo-terminal" >&2
    exit 1
fi

JZIP=$jzip STORY=$story TITLE=$title RELEASE=$release expect <<'EOF'
set timeout 15
log_user 0
expect_before timeout { exit 1 }
spawn env TERM=xterm "$env(JZIP)" "$env(STORY)"
expect "$env(TITLE)"
expect "$env(RELEASE)"
expect ">"
send "quit\r"
expect "Do you wish to leave the game?"
send "y\r"
expect "Hit any key to exit."
send "x"
expect eof
EOF
