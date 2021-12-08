#!/usr/bin/env bash
gcc -o hat.so hat.c $(yed --print-cflags) $(yed --print-ldflags)