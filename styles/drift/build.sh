#!/usr/bin/env bash
gcc -o drift.so drift.c $(yed --print-cflags) $(yed --print-ldflags)