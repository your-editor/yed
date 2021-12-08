#!/usr/bin/env bash
gcc -o jgraph.so jgraph.c $(yed --print-cflags) $(yed --print-ldflags)