#!/usr/bin/env bash
gcc -o sh.so sh.c $(yed --print-cflags) $(yed --print-ldflags)