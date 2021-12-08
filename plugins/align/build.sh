#!/usr/bin/env bash
gcc -o align.so align.c $(yed --print-cflags) $(yed --print-ldflags)