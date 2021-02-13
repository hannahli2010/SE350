# Group 10 P1 submission

## Directory structure
All the code from `Context_Switching` has been placed in the `code` folder.
Our test suites have been separated into the `tests` folder as `AE3` and `AE4` (named this way since we were given `AE1` and `AE2`).
The `src` folder has been included in the compile paths for the files in the `tests` folder so that they can access the required header files.

## Running Tests
When running our test suites, you must add the `TEST_2_MEM_BLK` flag which will ensure that there are only 2 memory blocks.
Be sure to only include one of the 2 `AE{3, 4}` folders in the target build at a time.

In the `AE3` test suite, we test context switching by setting a variable to the expected next process id and verifying that the correct next proccess is run.
In the `AE4` test suite, the ends by going to the null process for which we cannot explicity output test results.
It was instead verified manually via a printf statement (wrapped by the `DEBUG_0` flag).

For more detailed descriptions of what each test does, read the brief description at the top of each `ae_proc_{3, 4}.c` file.
