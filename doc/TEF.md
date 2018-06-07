# Test Execution Framework (TEF) - Specification

Version 0.5

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
The recommended approach is to execute all files/directories, according to the
`argv[0]` rules above, ignoring hidden files (beginning with `.`), in
alphanumerical order.

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

Amongst other things, this helps with test parametrization, ie. running
```
/a/b/permission/devfileperm
/a/b/permission/dirperm
/a/b/permission/fileperm
```
where `permission` is an executable file (test).

All arguments need to be normalized and fully resolved prior to any evaluation,
that is any `..` paths should be transcribed, `./` removed, etc. If the final
result leads above CWD, it must be trimmed to stay in CWD, ie. by removing all
leading `..` elements.

## Standard IO redirection and logging

When executing any executable, the runner redirects both stdout and stderr
to a file inside a `logs` directory in the CWD, creating the directory if it
doesn't exist.

Alternatively, if `TEF_LOGS` env variable exists, path constructed as
```
TEF_LOGS/TEF_PREFIX
```
is to be used instead of the `logs` directory, created if necessary, relative
to CWD. If `TEF_PREFIX` doesn't exist, it is defined as `/`, see below for
further details.

If the executable is in CWD, the log output file name matches the name of
the executed binary. If the executable is a runner inside a subdirectory,
a name of the subdirectory is used instead.

The output file is opened for appending (`O_APPEND`), not overwriting.

## Result reporting

Result reporting is done by runners to multiple destinations and is done by
every runner (every recursion level).

### Realtime reporting

If the runner process has a valid terminal (ie. on fd 0), it outputs the report
on it in the following format:
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
followed by a forward slash. If it doesn't exist, it is defined as a forward
slash.

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

The status may be colorized or bold/underline/etc. if the terminal supports it.

There may be any number of spaces/tabs between the status and the test name,
however at least one is mandatory.

## Reporting into a log

If the runner does not have a terminal or if its standard output (stdout) is
different from the connected terminal (`fstat()` on both fds, comparing `st_ino`
and `st_dev`), it outputs a simplified binary version of the format used for
terminal reporting.

This simplified version starts with a header and a version, followed by
nul-separated results (`\0` denotes a nul byte):
```
tefresults\01\0RUN first_executable\0PASS first_executable\0
```

The `TEF_PREFIX` variable is ignored, no `/` is prepended.

No color or other special formatting characters are added by the runner.

It's expected that a parent runner would redirect the output to a log file,
as it doesn't differentiate between a child runner vs any executable.
The log files can then be collected by a separate tool and the hierarchy
re-constructed.

The header needs to be written every time a runner is started, as it cannot
rely on seeking its stdout. It's expected that any log-parsing tool would
skip the duplicate headers inside one log file, like ie. gzip does.

## Additional functionality

See other related documents that expand on this specification to provide
a more user-friendly interface and richer feature set.

## Examples

See other documents for further examples explaining these basic concepts.

Also see the `examples` directory for practical deployment demonstration.
