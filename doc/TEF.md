# Test Execution Framework (TEF) - Specification

Version 0.6

## Unspecified/undefined behavior

Anything outside this specification is implementation-dependent.

This document declares the minimum mandatory interface, other documents then
supplement it with recommendations and rationale.

## Hierarchy

The suite consists of hiearchical "levels", separated by directory levels
(or virtual/imaginary levels), with each level having one "runner" responsible
for test execution and results reporting. The API between levels is UNIX/POSIX,
so the runners can be implemented in different programming languages and don't
rely on implementation specifics.

Symbolic links (files) are always dereferenced when accessing a directory or
executing a file.

The primary function of the test "runner" is to iterate over the contents of
the current working directory (CWD):

* If a directory is encountered, it makes an attempt to find an executable file
  matching the current basename of `argv[0]` in this directory and execute it
  with CWD of that directory

* If an executable file is encountered and it doesn't match the basename of
  `argv[0]`, it is executed

"Basename" being the last member of the path, usually a filename.

## Argument passing

When called without CLI arguments, the behavior of the runner is undefined.

When called with arguments, the runner must execute exactly the files or
directories specified by the arguments.

An argument specifies the executable file names or relative paths leading into
subdirectories. For example `a` or `a/b/c`.

Before executing a file or a runner inside a directory, the topmost level
of the path in the argument is stripped and the result is passed to the
executable/runner. If this would result in the argument being empty, it is
omitted.

For example:
```
/a/b/c/d/e

- runner executable 'builder' in 'a' runs with argument 'b/c/d'
- it finds that 'b' is a directory
- it looks for an executable in 'b' called 'builder'
- it runs the executable, passing 'c/d' to it
```

Note that argument stripping is not tied to the nature of the executed file,
it is done both for a runner in a subdirectory as well as an executable in the
current directory, ie.
```
/a/b

- runner executable 'builder' in 'a' runs with argument 'b/c/d'
- it finds that 'b' is an executable file
- it runs 'b', passing 'c/d' as an argument to it
```

All arguments need to be normalized and fully resolved prior to any evaluation,
that is any `..` paths should be transcribed, `./` removed, etc. If the final
result leads above CWD, it must be trimmed to stay in CWD, ie. by removing all
leading `..` elements.

## Result reporting

Result reporting is done by every runner (every recursion level).

If the runner process has stdin (fd 0) open for writing, such as when
it is connected to a tty, it outputs status reporting lines on it with
the following format:
```
STATUS executable_name
```
where STATUS is one of PASS (retval 0) or FAIL (retval non-0) for a finished
execution or RUN for an executable that is being started, meaning the log could
look like ie.
```
RUN first_executable
PASS first_executable
RUN second_executable
FAIL second_executable
```
The lines may be interleaved if the executables run in parallel. In that case,
the runner needs to flock() its active tty (ie. fd 0) to get exclusive access
for writing.

Since the runner has no notion of its location within the hierarchy, it needs
to prefix each executable name with the contents of `TEF_PREFIX` env variable,
followed by a forward slash. If the variable doesn't exist, it is defined as
a forward slash.

When executing a runner inside a subdirectory, the subdirectory name is appended
to the `TEF_PREFIX` variable passed to the runner, separated by a forward slash.

Thus, the actual results could look like ie.
```
RUN /first_executable
PASS /first_executable
RUN /subdir/second_executable
FAIL /subdir/second_executable
```
The first two lines being from a first runner, the last two lines from another
runner inside a `subdir` directory, to which the first runner passed
`TEF_PREFIX` equal to `/subdir`.

The status may be colorized or bold/underline/etc., but only if the output fd is
a terminal (`tcgetattr(3)`). Terminfo should be queried for safety.

There may be any number of spaces/tabs between the status and the test name,
however at least one is mandatory.

### Reporting on stdout and stderr

The runner shall use output stdio for any relevant debugging, informatory or
error output. The format is undefined here.

During normal and default operation, no output should be written to stdout or
stderr.

## Standard IO redirection and logging

When executing any executable, the runner redirects both stdout and stderr
to a log file. The log file is always opened for overwriting (without
`O_APPEND`).

### Logging in CWD

By default, the log files are placed inside a `logs` directory in CWD, which
is created if nonexistent. This way, each directory level has locally-accessible
logs, allowing execution to start from any point of the hierarchy.

The name of each log file matches the name of the executable. When executing
a subdirectory (executable with argv[0] name in a directory), the filename
inside `logs` is the name of the subdirectory, not argv[0].

### Logging in separate hierarchy

When `TEF_LOGS` env variable exists, the above logging scheme is replaced with
one that mimics the execution hierarchy, with `TEF_LOGS` specifying the root,
created if necessary.
As such, the destination logfile is stored in `TEF_LOGS/TEF_PREFIX`.

Since directories and log files from argv[0] runners of the directories cannot
share the same name, `.log` is appended to all log files inside the hierarchy.

Ie. under `TEF_LOGS`:
```
/first_executable.log
/subdir/second_executable.log
/subdir.log
```

## Additional functionality

See other related documents that expand on this specification to provide
a more user-friendly interface and richer feature set.

## Examples

See other documents for further examples explaining these basic concepts.

Also see the `examples` directory for practical deployment demonstration.
