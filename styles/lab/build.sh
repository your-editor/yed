#!/usr/bin/env bash
gcc -o lab.so lab.c $(yed --print-cflags) $(yed --print-ldflags)