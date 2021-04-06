# Group 10 P3 submission

## Directory structure
All of our system's code has been placed in the `code` folder.

Our user tests from P1 and P2 have been copied into the `tests` folder for posterity (but no changes were made for P3).

Note that the tests for P3 do NOT follow the usual testing format as user tests for P3 are optional, and so
we have included them only for completeness, and not for grading.

## Running Tests
To run a test, drag the test from one of `P1`, `P2`, or `P3` groups on uVision to the `currTest` group.
This `currTest` group has the `src` folder included in the compile paths so that the current test can access the required header files.
Make sure there is only one test in the `currTest` group at a time when compiling.

If you are running your own tests, make sure the test uses the functions defined in the `ae.h` file that was provided during `P2`. In other words, the test must have the `set_test_procs` that was added to the `ae.h` file from `P2` onwards.
  