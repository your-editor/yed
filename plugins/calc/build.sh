#!/usr/bin/env bash
gcc -o calc.so calc.c $(yed --print-cflags) $(yed --print-ldflags)