#!/usr/bin/env bash
gcc -o vt.so vt.c $(yed --print-cflags) $(yed --print-ldflags)