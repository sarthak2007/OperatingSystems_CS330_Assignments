#! /bin/sh

gcc part1.c
./a.out $1 $2 > a.txt
grep -rF $1 $2 > b.txt
diff a.txt b.txt
rm a.txt b.txt