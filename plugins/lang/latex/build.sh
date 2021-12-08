#!/usr/bin/env bash
gcc -o latex.so latex.c $(yed --print-cflags) $(yed --print-ldflags)