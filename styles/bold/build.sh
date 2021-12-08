#!/usr/bin/env bash
gcc -o bold.so bold.c $(yed --print-cflags) $(yed --print-ldflags)