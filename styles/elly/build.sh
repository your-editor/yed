#!/usr/bin/env bash
gcc -o elly.so elly.c $(yed --print-cflags) $(yed --print-ldflags)