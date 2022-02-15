# CS 526 Course Project I

kunli3

How to run test:

1. replace `LLVMROOT` in tests/Makefile

1. `mkdir build && cd build`

1. `cmake ..`

1. `make && cd ../tests`

2. run `make filename=${filename} test_pass`

   e.g. run `make filename=nested test_pass`. It will examine the transformation of struct in `nested.c` and examine the execution of compiled program. The output is like:

   ```
   ➜ make filename=nested test_pass
   /Users/likun/Projects/VSCodeProjects/Course/CS526/llvm-8.0.1/bin/clang -S -emit-llvm -o - nested.c | /Users/likun/Projects/VSCodeProjects/Course/CS526/llvm-8.0.1/bin/llvm-as -o=nested.llvm.bc
   /Users/likun/Projects/VSCodeProjects/Course/CS526/llvm-8.0.1/bin/opt < nested.llvm.bc | \
           /Users/likun/Projects/VSCodeProjects/Course/CS526/llvm-8.0.1/bin/opt -load ../build/pass/libSROA.so -scalarrepl-kunli3 -o=nested.test.bc
   /Users/likun/Projects/VSCodeProjects/Course/CS526/llvm-8.0.1/bin/llvm-dis -f nested.test.bc
   /Users/likun/Projects/VSCodeProjects/Course/CS526/llvm-8.0.1/bin/clang -Xclang -load -Xclang ../build/pass/libSROA.* nested.c \
                   && echo "[function output]": \
                   && ./a.out \
                   && echo "[SROA test]": \
                   && FILE_NAME=nested.test.ll python3 test.py
   [function output]:
   testNested: 10  
   [SROA test]:
   passed
   rm nested.llvm.bc
   ```
   
   