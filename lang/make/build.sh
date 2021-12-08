#!/usr/bin/env bash
gcc -o make.so make.c $(yed --print-cflags) $(yed --print-ldflags)