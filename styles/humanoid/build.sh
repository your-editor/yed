#!/usr/bin/env bash
gcc -o humanoid.so humanoid.c $(yed --print-cflags) $(yed --print-ldflags)