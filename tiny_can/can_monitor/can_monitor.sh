#!/bin/sh

PRG=can_monitor

STARTDIR=$(pwd)
APPDIR="${0%/*}"
cd "$APPDIR"
APPDIR=$(pwd)
cd "$STARTDIR"

export LD_LIBRARY_PATH=$APPDIR:$LD_LIBRARY_PATH:/usr/local/lib

$APPDIR/$PRG $@

