# Assignment 2 — Parse Tree Reduction to Normal Form

## Description
This program extends the Assignment 1 recursive descent parser for algebraic
expressions (based on the expression grammar from pages 459–460 of James
Hein's book). In addition to parsing an expression and printing its parse
tree, the program:

1. Parses the input expression and prints its parse tree.
2. Applies the reduction rules from page 460 to convert the parse tree
   step-by-step into its normal form, printing every intermediate parse
   tree along with its corresponding expression.
3. Prints the final normal-form parse tree and its corresponding expression.

The extended BNF grammar and reduction rules used are documented as
comments at the top of the source `.cpp` file.

## Build
```bash
g++ -O3 -o parser.exe parser.cpp
```

## Run
```bash
./parser.exe
```
The program reads an expression from standard input and prints the parse
tree, each intermediate reduction step (tree + expression), and the final
normal-form tree and expression.

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

product
|--inverse
   |--z
|--inverse
   |--product
      |--x
      |--inverse
         |--y
z^-1.(x.y^-1)^-1

product
|--inverse
   |--z
|--product
   |--inverse
      |--inverse
         |--y
   |--inverse
      |--x
z^-1.((y^-1)^-1.x^-1)

product
|--inverse
   |--z
|--product
   |--y
   |--inverse
      |--x
z^-1.(y.x^-1)
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
