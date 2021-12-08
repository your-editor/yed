#!/usr/bin/env bash
gcc -o first.so first.c $(yed --print-cflags) $(yed --print-ldflags)