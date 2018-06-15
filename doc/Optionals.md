# Optional functionality and examples

## Argument combining/merging

(Optional feature.)

To run more tests while sharing their setup/cleanup, you can specify more
arguments to the runner binary.

The runner can use a guessing logic to strip a common leading path as it
enumerates the arguments until it encounters an argument where the initially
found leading path prefix cannot be found. Then it passes the list of arguments
(enumerated so far) to a sublevel runner, as arguments.

For example:
```
/a/b/c/d/e

- runner executable 'builder' in 'a' runs with arguments 'b/c/d' 'b/c/x'
- it figures out that 'b/c' is the shared leading prefix
- it executes 'b/builder', passing 'c/d' and 'c/x' to it
- the runner in 'b' figures out that 'c' is the leading prefix
- it executes 'c/builder', passing 'd' and 'x' to it
```
Without the merging logic, the entire chain from 'a' through 'b' would be
executed twice, once for each argument.

The merging can be performed only on subsequent arguments, eg. the arguments
must not be reordered as that would alter the execution flow.

## Log file rotation

(Optional feature.)

As the log file of each test is overwritten on suite re-runs, the runner may
implement log file rotation to preserve previous versions of the log.

This would be done in the `logrotate(8)` fashion, eg. by appending `.1` to the
existing filename and if it already exists, renaming the existing `.1` to `.2`,
etc., up until a defined maximum of ie. 8 files total.

This obviously risks mixing unrelated test results if there are two separate
tests with one having the exact same name as the other, but with `.1` appended.
