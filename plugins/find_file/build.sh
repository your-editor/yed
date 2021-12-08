#!/usr/bin/env bash
gcc -o find_file.so find_file.c $(yed --print-cflags) $(yed --print-ldflags)