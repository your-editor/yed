#!/usr/bin/env bash
gcc -o river.so river.c $(yed --print-cflags) $(yed --print-ldflags)