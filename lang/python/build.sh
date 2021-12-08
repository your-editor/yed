#!/usr/bin/env bash
gcc -o python.so python.c $(yed --print-cflags) $(yed --print-ldflags)