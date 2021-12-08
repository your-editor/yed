#!/usr/bin/env bash
gcc -o skyfall.so skyfall.c $(yed --print-cflags) $(yed --print-ldflags)