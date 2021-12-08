#!/usr/bin/env bash
gcc -o log_hl.so log_hl.c $(yed --print-cflags) $(yed --print-ldflags)