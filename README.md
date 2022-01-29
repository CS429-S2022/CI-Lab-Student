# CI-lab

This is the code base for CS429 F2022 CI lab. Check the writeup for more details.

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
 node.h
 parse.c
 print.c
 token.h
 type.h
 update.sh
 value.h
 variable.c
 variable.h
 ```
 * Students are required to complete `eval.c` and `variable.c`. 

 * `ci_reference` is a compiled reference program executable provided to students. `driver.sh <filename>` runs `ci` and `ci_reference` and compares their outputs.

 * `tests/` holds the testcases provided to students. Students are required to pass all testcases in `test_week1.txt` for week 1.

## Program
* Run `make ci` to get the binary `ci`. Run `make test_week#` to run the simple grader on a given week's test cases.

* `ci` takes two optional arguments: 
    ```
    ./ci -i <infile> -o <outfile>
    ```
    The infile can also be fed from stdin with `./ci < <infile>`. The outfile is only used for grading, where the formatting stuffs are removed.

* The interactive prompt of `ci` takes two special commands:
    * `@q` to quit the program
    * `@p` to print all defined variables (EEL-2)
