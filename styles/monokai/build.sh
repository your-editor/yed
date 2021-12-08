#!/usr/bin/env bash
gcc -o monokai.so monokai.c $(yed --print-cflags) $(yed --print-ldflags)