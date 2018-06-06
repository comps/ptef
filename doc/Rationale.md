# Reasoning behind specific design decisions

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
