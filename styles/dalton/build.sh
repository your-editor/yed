#!/usr/bin/env bash
gcc -o dalton.so dalton.c $(yed --print-cflags) $(yed --print-ldflags)