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

output=$(printf 'quit\ny\n' | "$jzip" "$story")

case $output in
    *"$title"*) ;;
    *) echo "Expected story title not found: $title" >&2; exit 1 ;;
esac

case $output in
    *"$release"*) ;;
    *) echo "Expected story release not found: $release" >&2; exit 1 ;;
esac
