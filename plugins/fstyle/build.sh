#!/usr/bin/env bash
gcc -o fstyle.so fstyle.c $(yed --print-cflags) $(yed --print-ldflags)
