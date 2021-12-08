#!/usr/bin/env bash
gcc -o tempus_future.so tempus_future.c $(yed --print-cflags) $(yed --print-ldflags)