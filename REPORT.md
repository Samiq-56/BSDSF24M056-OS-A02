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

## Feature 5: ls-v1.4.0 - Alphabetical Sort

### Q1: Why is it necessary to read all directory entries into memory before you can sort them? What are the potential drawbacks of this approach for directories containing millions of files?
**Answer:**
- **Why it is necessary:** Sorting requires comparing elements against each other and rearranging their positions. Since a directory stream provides entries sequentially as they are stored on disk (or read by the file system), you cannot sort them on-the-fly without first storing them in a data structure (such as a dynamic array of pointers in memory) where random access and comparison functions like `qsort()` can be applied.
- **Potential drawbacks:** For directories containing millions of files, loading every entry into memory requires a massive amount of RAM. This can lead to memory exhaustion (out-of-memory errors) or significant performance degradation due to memory allocation overhead and cache misses.

### Q2: Explain the purpose and signature of the comparison function required by qsort(). How does it work, and why must it take const void * arguments?
**Answer:**
- **Purpose and Signature:** The comparison function tells `qsort()` how to order the elements. Its standard signature takes two `const void *` pointers: `int (*compar)(const void *, const void *)`.
- **How it works:** It casts these generic pointers to the actual data type being sorted (in this case, `char **` for an array of strings), compares them using a function like `strcmp()`, and returns an integer less than, equal to, or greater than zero depending on whether the first element is less than, equal to, or greater than the second.
- **Why `const void *`:** `qsort()` is a generic sorting algorithm in the C standard library designed to sort *any* data type (integers, floats, structs, strings, etc.). By accepting `const void *`, it maintains type-safety independence, allowing the same sorting routine to handle any custom data structure while ensuring the comparison function cannot accidentally modify the underlying data.


## Feature 6: ls-v1.5.0 - Colorized Output Based on File Type

### Q1: How do ANSI escape codes work to produce color in a standard Linux terminal? Show the specific code sequence for printing text in green.
**Answer:**
- 

### Q2: To color an executable file, you need to check its permission bits. Explain which bits in the st_mode field you need to check to determine if a file is executable by the owner, group, or others.
**Answer:**
-
