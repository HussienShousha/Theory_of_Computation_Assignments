# Assignment 1 — Recursive Descent Parser

## Description
This program implements a recursive descent parser for algebraic expressions
(based on the expression grammar from pages 459–460 of James Hein's book).
The parser reads an input expression, builds a parse tree, and prints the
tree in a nested indented format.

The extended BNF grammar used is documented as a comment at the top of the
source `.cpp` file.

## Build
```bash
g++ -O3 -o parser.exe parser.cpp
```

## Run
```bash
./parser.exe
```
The program reads an expression from standard input (or a file, depending
on implementation) and prints the resulting parse tree.

## Example

**Input:**
```
((x.y^-1).z)^-1
```

**Output:**
```
inverse
|--product
   |--product
      |--x
      |--inverse
         |--y
   |--z
```

## Constraints followed
- Only `<cstdlib>`, `<cstdio>`, `<cstring>`, `<iostream>` are used.
- No STL containers or `std::string`.
- Code is under 15,000 characters.
- No TAB characters (converted to 4 spaces).
- Coding style follows `CodingStyle.pdf`.

## Submission
The final `.cpp` file was encoded using `assign_encoder.cpp` as required by
the assignment instructions before submission via the course Google Form.
