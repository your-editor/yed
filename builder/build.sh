#!/usr/bin/env bash
gcc -o builder.so builder.c $(yed --print-cflags) $(yed --print-ldflags)