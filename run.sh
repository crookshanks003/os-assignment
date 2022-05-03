#!/bin/bash

g++ scheduler.c lib.c -o sche.out -pthread
g++ p1.c lib.c -o p1.out -pthread
g++ p2.c lib.c -o p2.out -pthread

./sche.out
