#!/usr/bin/env bash
gcc -o embark.so embark.c $(yed --print-cflags) $(yed --print-ldflags)