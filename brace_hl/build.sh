#!/usr/bin/env bash
gcc -o brace_hl.so brace_hl.c $(yed --print-cflags) $(yed --print-ldflags)