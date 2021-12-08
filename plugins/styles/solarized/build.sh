#!/usr/bin/env bash
gcc -o solarized.so solarized.c $(yed --print-cflags) $(yed --print-ldflags)