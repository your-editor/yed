#!/usr/bin/env bash
gcc -o doug.so doug.c $(yed --print-cflags) $(yed --print-ldflags)