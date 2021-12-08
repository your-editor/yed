#!/usr/bin/env bash
gcc -o casey.so casey.c $(yed --print-cflags) $(yed --print-ldflags)