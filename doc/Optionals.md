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

## Path relativity

(Optional feature.)

The runner is aware of its on-disk location from the dirname of `argv[0]` and
its relation to CWD as written in the spec. This feature overrides the default
exit-with-non-0 behavior when the dirname differs from CWD.

If CWD is parent to the runner's location, the runner must exit immediately with
a non-0 return value as it would be working with files outside its scope.

If CWD is equal to the runner's location, no special rules apply.

If the runner's location is parent to CWD, the runner needs to re-construct the
path to CWD from its location and run as if the user passed this path as
argument.

For example:
```
/a/b/c/d/e

- runner executable 'builder' in 'a'
- user in '/a/b/c' as CWD runs '../../builder'
- runner in 'a' figures out that the path to CWD is 'b/c'
- runner in 'a' finds out that 'b' is a directory
- runner in 'a' executes 'b/builder', passing it 'c' as argument
- runner in 'b' finds out that 'c' is a directory
- runner in 'b' executes 'c/builder', passing no arguments
```
The result is the same as if the user executed the runner in CWD, but with the
parent runners also being involved and ie. performing suite-wide setup/cleanup
that's outside the scope of the CWD runner.

### Argument relativity

Any arguments passed to a runner shall be treated as relative to CWD, not the
runner's location. This means that, like with path relativity, arguments need
to be adjusted to account for this prior to execution.

For example:
```
/a/b/c/d/e

- runner executable 'builder' in 'a'
- user in '/a/b/c' as CWD runs '../../builder d/e'
- runner in 'a' figures out that the path to CWD is 'b/c'
- runner in 'a' prepends that path to its arguments, 'd/e' becomes 'b/c/d/e'
- runner in 'a' finds out that 'b' is a directory
- runner in 'a' executes 'b/builder', passing it 'c/d/e' as argument
- runner in 'b' finds out that 'c' is a directory
- runner in 'b' executes 'c/builder', passing it 'd/e'
- ...
```
The result is the same as if the user executed the runner in CWD and passed it
the same arguments as to the parent runner, with the parent runners being
involved in setup/cleanup or other tasks.

If an absolute path, denoted by the first character being `/`, is passed to any
runner, the runner shall use the argument verbatim, treating its own (runner's)
location as the root, effectively ignoring CWD.

