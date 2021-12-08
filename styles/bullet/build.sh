#!/usr/bin/env bash
gcc -o bullet.so bullet.c $(yed --print-cflags) $(yed --print-ldflags)