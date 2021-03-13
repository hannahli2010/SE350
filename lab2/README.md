# Group 10 P2 submission

## Directory structure
All the code from `Context_Switching_IPROC` has been placed in the `code` folder.
Our test suites have been separated into the `tests` folder as `AE[06-10]` (named this way since we were given `AE[1-2]`, and `AE[3-5]` were used for the P1 submission).
The `src` folder has been included in the compile paths for the files in the `tests` folder so that they can access the required header files.

## Running Tests
When running test suite `AE07`, you must add the `TEST_2_MEM_BLK` flag which will ensure that there are only 2 memory blocks.
Every other test suite (`AE06`, `AE08`, `AE09`, `AE10`) must **not** have the `TEST_2_MEM_BLK` flag enabled.
Be sure to only include one of the `AE[06-10]` folders in the target build at a time.

In our test suites, we test context switching by setting a variable to the expected next process id and verifying that the correct next proccess is run.
When testing message passing, to simplify testing we only assert the validity of the first character in each message.

For more detailed descriptions of what each test does, read the brief description at the top of each `ae_proc_[06-10].c` file.
