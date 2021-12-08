#!/usr/bin/env bash
gcc -o line_numbers.so line_numbers.c $(yed --print-cflags) $(yed --print-ldflags)