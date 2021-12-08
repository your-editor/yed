#!/usr/bin/env bash
gcc -o forest.so forest.c $(yed --print-cflags) $(yed --print-ldflags)