#!/usr/bin/env bash
gcc -o elise.so elise.c $(yed --print-cflags) $(yed --print-ldflags)