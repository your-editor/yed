#!/usr/bin/env bash
gcc -o blue.so blue.c $(yed --print-cflags) $(yed --print-ldflags)