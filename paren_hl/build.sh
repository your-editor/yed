#!/usr/bin/env bash
gcc -o paren_hl.so paren_hl.c $(yed --print-cflags) $(yed --print-ldflags)