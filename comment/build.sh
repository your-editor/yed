#!/usr/bin/env bash
gcc -o comment.so comment.c $(yed --print-cflags) $(yed --print-ldflags)