#!/bin/bash
SRC=/home/smith/workspace/Claude/assistkit/claude/commands
DST=/home/smith/.claude/commands

mkdir -p $DST
cp $SRC/*.md $DST/
echo "Synced $(ls $SRC/*.md | wc -l) commands to $DST"
