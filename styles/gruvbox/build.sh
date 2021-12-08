#!/usr/bin/env bash
gcc -o gruvbox.so gruvbox.c $(yed --print-cflags) $(yed --print-ldflags)