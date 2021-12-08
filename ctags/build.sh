#!/usr/bin/env bash
gcc -o ctags.so ctags.c $(yed --print-cflags) $(yed --print-ldflags)