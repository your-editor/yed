#!/usr/bin/env bash
gcc -o acme.so acme.c $(yed --print-cflags) $(yed --print-ldflags)