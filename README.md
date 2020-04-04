# Socketeer
Socketeer is a program aimed at sending data across platforms, my first program written in C! It supports Windows, macOS and Linux.


## Prerequisites and Compilation
Socketeer can be compiled on any platform using `clang` or `gcc`, modify the Makefile's compiler as you wish. To compile using the regular compiler:
```
make
```
To compile for Windows (assuming you have setup a cross-compiler already and modified the compiler in the Makefile):
```
make OS=win
```
