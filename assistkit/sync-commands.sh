#!/bin/bash
SRC="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/claude/commands"
DST=/home/smith/.claude/commands

mkdir -p $DST
cp $SRC/*.md $DST/
echo "Synced $(ls $SRC/*.md | wc -l) commands to $DST"
