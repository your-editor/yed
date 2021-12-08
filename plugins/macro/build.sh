#!/usr/bin/env bash
gcc -o macro.so macro.c $(yed --print-cflags) $(yed --print-ldflags)