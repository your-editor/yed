#!/usr/bin/env bash
gcc -o c.so c.c $(yed --print-cflags) $(yed --print-ldflags)