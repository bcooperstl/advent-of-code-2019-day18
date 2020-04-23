#!/bin/bash
gcc -g -c cache.c
gcc -g -c map.c
gcc -g -c part1.c
gcc -g -c part2.c
gcc -o part1 part1.o map.o cache.o
gcc -o part2 part2.o map.o cache.o

