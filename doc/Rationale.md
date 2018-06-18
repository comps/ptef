# Reasoning behind specific design decisions

## Treating directories as executables

The central execution logic revolves around making no distinction when executing
a regular executable file and an argv[0]-named executable inside a directory.
This might seem odd as it could be presumed that a subdirectory would always
have its own "runner" and any regular executable would always be a simple test.

Given this distinction, we could make smart decisions of not passing `TEF_*`
variables to simple tests while ie. not reporting results for runners.

However treating both the same allows extending the logic above filesystem
concepts, such as when running 3rd party suites via a single runner that
imitates the filesystem structure virtually (ie. for Linux Test Project).

It also permits passing "parameters" to tests (regular executables) by treating
the executable like a runner and allowing it to report results for each
parameter separately.

Similarly, it allows each test to be in its own directory, if necessary
(bundling LICENSE, README, multiple execution modes per test, etc.) without
needing a runner inside each directory.

In the end, there is not much benefit to be gained from splitting the logic
and implementation and specification complexity would suffer too.

## Environment variables, not CLI options

While POSIX.1-2008 defines some guidelines for CLI option format, namely
http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html,
and while many programming languages support `POSIXLY_CORRECT`, the CLI
interface is hardly standard:

* Despite the POSIX recommendations, many languages implement them differently
  in subtle ways (ie. silently treating `-1` as option argument)

* Some languages implement more programmer-friendly interfaces compared to
  their getopt-based implementation, thus discourating getopt use

* POSIX specifies only single-letter options, thus limiting the namespace
  and causing potential conflicts

* Options with non-trivial arguments (whitespaces, newlines) are hard to pass
  correctly

* Each level of the runner would need to pass all arguments to sub-runners,
  including the actual tests, which would need to ignore the arguments

Thus, all settings, paths, features and switches are passed between runner
levels using environment variables with known names.

Note that a runner implementation can provide user-friendly CLI interface,
unspecified here, but it itself needs to export the user-provided settings
via env variables to sub-runners.

## Running without arguments is undefined

When running without arguments, the recommended approach is to execute all
files/directories, according to the `argv[0]` rules in the spec, ignoring
hidden files (beginning with `.`), in alphanumerical order.

However that may not fit all cases, such as when multiple runners are present
on the same directory level, with different argv[0] names, ie. 'setup', 'run',
'cleanup', each recursing through the structure, but only 'run' executing
binaries which don't match argv[0].
