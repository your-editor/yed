#!/usr/bin/env bash
gcc -o vimish.so vimish.c $(yed --print-cflags) $(yed --print-ldflags)