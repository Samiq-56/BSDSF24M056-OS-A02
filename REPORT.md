# Project Report: Custom 'ls' Utility Clone

## 1. Introduction
This project implements a robust clone of the standard Linux `ls` utility in C, supporting dynamic memory allocation, permission parsing, `ioctl` terminal width detection, and columnar formatting.

## 2. Technical Core Logic
- **Down-Then-Across Format:** Requires 2D grid index calculation (`c * num_rows + r`) because a simple linear loop cannot handle vertical column filling.
- **ioctl System Call:** Queries terminal width dynamically (`TIOCGWINSZ`) to handle resizing gracefully, avoiding the limitations of a fixed 80-column fallback.

## 3. Compilation & Usage
```bash
make
./bin/ls
./bin/ls -l
./bin/ls -x
```
