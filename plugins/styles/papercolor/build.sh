#!/usr/bin/env bash
gcc -o papercolor.so papercolor.c $(yed --print-cflags) $(yed --print-ldflags)