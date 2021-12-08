#!/usr/bin/env bash
gcc -o olive.so olive.c $(yed --print-cflags) $(yed --print-ldflags)