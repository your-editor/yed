#!/usr/bin/env bash
gcc -o indent_c.so indent_c.c $(yed --print-cflags) $(yed --print-ldflags)