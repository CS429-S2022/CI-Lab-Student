# CI-lab

This is the internal code base for CS429 SP21 CI lab. Check the writeup for more details.

 ## Files
 Files should be included in handout (`make handout`):
 ```
 tests/
 ansicolors.h
 ci_reference
 ci.c
 ci.h
 driver.sh
 err_handler.c
 err_handler.h
 eval.c
 handle_args.c
 interface.c
 lex.c
 Makefile
 nodes.h
 parse.c
 print.c
 token.h
 type.h
 value.h
 variable.c
 variable.h
 ```
 * Students are required complete `parse.c` and `eval.c` (and also `variable.c` for EEL-2). The reference solution are in `parse-ref.c`, `eval-ref.c` and `variable-ref.c`.

 * `ci_reference` is a compiled reference program provided to students. `driver.sh <filename>` runs `ci` and `ci_reference` and compare their outputs.

 * `tests/` holds the testcases provided to students. Students are required to pass all testcases in `test_simple.txt` for checkpoint 1. They are asked to create their own testcases in `tests/test_custom.txt` for checkpoint 2. The other testcases are categorized loosely by expression types. `tests-ref/` holds the testcases for final grading.

## Program
* Run `make ci` to get the binary `ci`. Run `make test TESTS=<filename>` to run the simple grader on a test file. `make test` runs `tests/test_simple.txt` by default. `./driver.sh <filename>` has the same effect.

* `ci` takes two optional arguments: 
    ```
    ./ci -i <infile> -o <outfile>
    ```
    The infile can also be fed from stdin with `./ci < <infile>`. The outfile is only used for grading, where the formatting stuffs are removed.

* The interactive prompt of `ci` takes two special commands:
    * `@q` to quit the program
    * `@p` to print all defined variables (EEL-2)

## Autograder
Use `make autograder` to create the zip file for gradescope grading. Remove the prefix `_` from checkpoint test scripts to activate them.
