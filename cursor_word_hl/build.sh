#!/usr/bin/env bash
gcc -o cursor_word_hl.so cursor_word_hl.c $(yed --print-cflags) $(yed --print-ldflags)