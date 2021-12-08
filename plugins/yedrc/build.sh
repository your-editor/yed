#!/usr/bin/env bash
gcc -o yedrc.so yedrc.c $(yed --print-cflags) $(yed --print-ldflags)
