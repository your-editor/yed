#!/usr/bin/env bash
gcc -o moss.so moss.c $(yed --print-cflags) $(yed --print-ldflags)