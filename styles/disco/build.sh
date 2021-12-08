#!/usr/bin/env bash
gcc -o disco.so disco.c $(yed --print-cflags) $(yed --print-ldflags)