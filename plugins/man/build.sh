#!/usr/bin/env bash
gcc -o man.so man.c $(yed --print-cflags) $(yed --print-ldflags)
