#!/usr/bin/env bash
gcc -o book.so book.c $(yed --print-cflags) $(yed --print-ldflags)