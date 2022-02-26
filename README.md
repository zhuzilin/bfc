# bfc

A really fast non-JIT brainfuck interpreter. It is 60% faster than the `bf2run` interpreter in [rdebath/Brainfuck](https://github.com/rdebath/Brainfuck) for the classic mendelbrot benchmark:

||mendelbrot time/s||
|-|-|-|
|bf2run|2.46||
|**bfc**|**1.53**|**1.61x**|

Tested on MacBook Pro 2019, Intel Core i7 2.6GHz.

## compile and run

`src/interpreter.cc` contains all the source code of bfc, to compile:

```bash
g++ -std=c++11 -O2 src/interpreter.cc -o bfc
```

To run the mendelbrot benchmark:

```bash
./bfc bf/mendelbrot.bf
```

## tricks in bfc

You may wonder why bfc is this fast. This repo also contains some intermediate interpreter implementation:

- naive: A straightforward implementation of the [commands in wiki](https://en.wikipedia.org/wiki/Brainfuck#Commands). Code in `src/naive.cc`.
- switch threading: Merge the consequent commands, e.g. `+++` to `*ptr += 3`. Code in `src/switch.cc`.
- direct threading: Use [direct threading](https://en.wikipedia.org/wiki/Threaded_code#Direct_threading) instead of switch threading. This relies the [labels as values](https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html) extension of GNU C. Code in `src/direct.cc`.
- bfc: Some peephole optimization on top of `src/direct.cc`, especially loop optimization, e.g. `[-]` to `*ptr = 0`. Code in `src/interpreter.cc`.

Here is the performance comparison of them:

||mendelbrot time/s||command|
|-|-|-|-|
|naive|23.5||`g++ -std=c++11 -O2 src/naive.cc -o bfc`|
|switch threading|6.58|3.57x|`g++ -std=c++11 -O2 src/switch.cc -o bfc`|
|direct threading|3.05|7.70x|`g++ -std=c++11 -O2 src/direct.cc -o bfc`|
|**bfc**|**1.53**|**15.4x**|`g++ -std=c++11 -O2 src/interpreter.cc -o bfc`|

You could compare the code to see how bfc is 15 times as fast as the naive implementation:)

It's also worth to notice that, as a non-JIT interpreter, bfc is still much slower than a naive compiler implementation.

||mendelbrot time/s||
|-|-|-|
|naive compiler|0.87||
|**bfc**|**1.53**|**0.57**|

The naive compiler is implemented in `src/compiler.cc`, it will compile brainfuck into C. To use the compiler, run:

```bash
g++ -std=c++11 -O2 src/compiler.cc -o compiler
./compiler bf/mendelbrot.bf > mendelbrot.c
gcc -O2 mendelbrot.c -o mendelbrot
./mendelbrot
```

## TODO

- [ ] Add JIT, may be using [xbyak](https://github.com/herumi/xbyak).
