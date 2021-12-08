#!/usr/bin/env bash
gcc -o dracula.so dracula.c $(yed --print-cflags) $(yed --print-ldflags)