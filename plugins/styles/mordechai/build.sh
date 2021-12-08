#!/usr/bin/env bash
gcc -o mordechai.so mordechai.c $(yed --print-cflags) $(yed --print-ldflags)