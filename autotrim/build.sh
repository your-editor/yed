#!/usr/bin/env bash
gcc -o autotrim.so autotrim.c $(yed --print-cflags) $(yed --print-ldflags)