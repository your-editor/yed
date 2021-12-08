#!/usr/bin/env bash
gcc -o shell_run.so shell_run.c $(yed --print-cflags) $(yed --print-ldflags)