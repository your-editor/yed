#!/usr/bin/env bash
gcc -o grep.so grep.c $(yed --print-cflags) $(yed --print-ldflags)