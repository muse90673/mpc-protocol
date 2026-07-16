#!/bin/bash -ex
x86_64-w64-mingw32-gcc -o mpc-protocol.exe mpc-protocol.c -mwindows -municode -lshlwapi -lwininet -O2 -s
# aarch64-w64-mingw32-gcc -o mpc-protocol-arm64.exe mpc-protocol.c -mwindows -municode -lshlwapi -lwininet -O2 -s
