#!/usr/bin/env bash
gcc -o cpp.so cpp.c $(yed --print-cflags) $(yed --print-ldflags)