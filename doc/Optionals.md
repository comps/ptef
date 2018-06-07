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
