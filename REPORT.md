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

## 4. Feature-5: Alphabetical Sort (v1.4.0)

### Q1: Why read all directory entries into memory before sorting? What are the drawbacks for directories with millions of files?
- **Necessity:** Standard sorting algorithms (like `qsort()`) require random access to the dataset (elements in contiguous memory or pointers pointing to them) so elements can be compared and swapped efficiently. A directory stream (`DIR*`) read via `readdir()\ ettiği
iha stream-based hai, jismein ek waqt mein sirf ek entry milti hai aur random access mumkin nahi hota. Isliye, saari entries ko dynamic array mein load karna zaroori hai.
- **Drawbacks for millions of files:** 
  1. **High Memory Consumption:** Storing millions of file names and metadata pointers in RAM can exhaust available memory (RAM overflow/OOM).
  2. **Performance Bottleneck:** Allocating and reallocating memory dynamically for millions of items can slow down execution.

### Q2: Purpose and signature of the comparison function required by qsort()
- **Purpose & Signature:** The signature is `int compare_strings(const void *a, const void *b)`. Its purpose is to provide `qsort()` with custom logic on how to compare two elements of an unknown data type.
- **How it works:** It casts the `const void *` pointers to double pointers (`const char **`), dereferences them to get the actual strings, and uses `strcmp()` to determine their alphabetical order (returning negative, zero, or positive integers).
- **Why const void * arguments:** `qsort()` is a generic sorting function in C designed to sort *any* data type (integers, floats, structs, strings). By accepting `const void *`, it remains type-agnostic and can handle pointers to any memory location safely without modifying the original data.
