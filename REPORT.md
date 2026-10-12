# Project Report: Custom 'ls' Utility Clone (v1.3.0)

## 1. Introduction
This project implements a clone of the standard Linux `ls` utility in C, supporting dynamic memory allocation, long listing (-l), horizontal row-major layout (-x), and default vertical columnar formatting with dynamic terminal width detection via ioctl.

## 2. Report Questions & Technical Answers

### Q1: Complexity of 'Down Then Across' vs 'Horizontal' Printing Logic
- **Down-then-across (Vertical):** Requires significantly more pre-calculation. It must compute the number of rows and columns beforehand and use a 2D grid index mapping formula (c * num_rows + r) during printing.
- **Across (Horizontal):** Much simpler. It iterates linearly through filenames, tracks a running horizontal position, and wraps to a new line when the terminal width limit is exceeded.

### Q2: Display Mode Management Strategy
- **State Management:** Used state flag variables (long_format and horizontal_format) inside main.
- **Argument Parsing:** GNU getopt() parses command-line options (-l and -x).
- **Function Routing:** An if-else conditional block in do_ls routes to print_long_format(), print_horizontal_format(), or print_vertical_format().

## 3. Compilation & Usage
make
./bin/ls
./bin/ls -l
./bin/ls -x
