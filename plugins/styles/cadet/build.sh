#!/usr/bin/env bash
gcc -o cadet.so cadet.c $(yed --print-cflags) $(yed --print-ldflags)