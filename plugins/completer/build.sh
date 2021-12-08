#!/usr/bin/env bash
gcc -o completer.so completer.c $(yed --print-cflags) $(yed --print-ldflags)