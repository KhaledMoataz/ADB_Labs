## How to compile your code

1 - Create a build directory inside your porject directory and `cd` into it. \
2 - Run `cmake ..` to generate your make files.\
3 - Run `make` to compile your code - run it on each code change.

## How to run the test cases

1 - Run `./<executable_name> <test_file_path> <database_file_path> > <logs_file_path>` \
    Ex: `./chaining ../2nd_testcase.in chaining2.out > chaining2.log`

Note: don't forget to remove the old database on each new run using `rm <database_file_path>`
