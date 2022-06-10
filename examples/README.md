# Examples

This directory contains practical showcases of PTEF usage in made-up test
suites, which however demonstrate concepts useful in real test suites.

In order for the examples to be as realistic as possible, they use real paths
of `ptef-*` utilities and some of them symlink from `/usr/bin/ptef-runner`.
This means **you need PTEF installed on your system** (`make install` or via
an RPM) for these to work (in case you want to actually run them).

Despite this, the examples do not modify the installed system and can be
run unprivileged.

## Dependencies

On top of PTEF itself, you will need

```sh
dnf -y install diffutils xz
```

## How to run them

The examples are numbered - from the most trivial one (you should start with)
to the most complex one. Unless otherwise specified below, run each example
by issuing `./run` in its directory.

Use the `tree` utility (as `tree -C`) to view the hierarchy of each example.

## What they are

### Part 1: Hierarchy

Unlimited hierarchy levels as standard.

* `00-trivial`
  * A very simple example showing how execution propagates through the
    hierarchy and how subdirectories without a runner (`subdir1/`) stop
    the recursion.
* `01-symlinks`
  * A similarly simple showcase that illustrates how a runner follows
    symlinks to both executables and subdirectories.
  * Like the first example, it also shows how logs are stored (in `logs`
    on each hiearchy level) and rotated (on repeated execution).
    * Note how a runner inside a subdirectory is still treated like a regular
      executable and its stderr is logged as such.
* `02-test-lib`
  * This example demonstrates the use of a simple shared test library,
    created outside the scope of PTEF. This shows that PTEF is just one
    piece of the overall "test suite" puzzle and how it can smootly
    integrate with usecase-specific testing logic.
  * The test library is thus completely independent on PTEF, and PTEF
    is completely independent of it. The only shared thing is the directory
    hierarchy.
  * Note that, since `utils/` does not have a runner, none of its contents
    are executed by the runner.
* `03-shared-runner`
  * This demonstrates how you can use a bash-based runner wrapper in `utils/`
    and symlink it (instead of `/usr/bin/ptef-runner`) across your hierarchy,
    using the wrapper on every level.
  * The specific wrapper here does `chmod +x *.sh` before executing
    `ptef-runner`, but it obviously can do anything you need - setup/cleanup,
    wrapping everything in `valgrind`, etc., etc.
* `04-build-systems`
  * This shows, similarly to `02-test-lib`, how the same hiearchy can accomodate
    multiple recursive systems - ie. GNU Make (via `Makefile`s) alongside PTEF.
  * Do `make` before issuing `./run`.
* `05-executable-systems`
  * Like `04-build-systems`, but using executable files to build a test suite.
  * The challenge here is *not* running executable files from the build system,
    which are not part of the test set - this can be done by ie. the `-i` flag
    to `ptef-runner`.
  * Do `./build` before issuing `./run`.
* `06-runner-features` (same hierarchy as `00-trivial`)
  * This example exists only so that you can try out using runner in different
    non-default ways.
  * Try `./run` inside `subdir2/` to run a subtree of the test suite.
  * Try `./bar` inside `subdir2/` to run a single test, bypassing the runner.
  * Try `./run bar` inside `subdir2/` to run the same test, but via the runner.
    * (In some cases, a runner might provide setup/cleanup for tests.)
  * Try `./run -v bar` to not redirect stderr to a log (will leave it on
    console).
    * (This is great for debugging when you need to run test(s) via the runner.)
  * Try `./run -v` on the top level to run all tests verbosely (without stderr
    being redirected to a log).
    * This is the same as `PTEF_NOLOGS=1 ./run`.
  * Try `./run -r` on the top level to show `RUN` results.
    * (Useful as an indication when running tests that take a long time.)
    * This is the same as `PTEF_RUN=1 ./run`.
  * Try `exec 10>execlog; PTEF_RESULTS_FD=10 ./run; exec 10>&-` on the top level
    to create an extra unformatted copy of the results.
    * (Stdout is not safe as many tools might accidentally write to it, whereas
      `PTEF_RESULTS_FD` guarantees a fixed format.)
    * Observe via `cat execlog`, or search for `grep ^FAIL execlog`.
* `07-arguments`
  * Do `./run /subdir1/deepdir` to run only that specific subtree.
    * (You can also pass `subdir1/deepdir/`, via tab-completion.)
  * Do `./run subdir1/deepdir/[23]*` to run only specific tests.
    * (This will pass 2 arguments and traverse the hierarchy only once.)
  * Do `./run -m subdir1/deepdir/[23]*` to run each test separately.
    * (This will traverse the hierarchy twice, once per each argument.)
  * Do `./run -d /subdir1/deepdir` to spawn a shell inside `deepdir`.
    instead of running any executables (remember to `exit` from it).
    * (Useful for hand-debugging individual tests that share a lengthy setup.)
  * Note that the "Doing setup/cleanup ..." printouts were done by `run`
    (wrapping `ptef-runner`), but any test has access to stdin/stdout (as PTEF
    redirects only stderr), allowing you to write manual tests that interact
    with the user.
    * (Or simply test that provide verbose reasons for failure on stdout.)
* `08-logdir`
  * This demonstrates the use of a dedicated log directory (via `PTEF_LOGS`),
    which mimics the execution hierarchy, as a replacement for per-level `logs`
    directories.
  * Useful for log archival (just one directory instead of many `logs` ones).
* `09-parallel`
  * The scripts in this directory sleep for 1 second, so when executed normally
    (sequentially) take 5 seconds to complete.
  * Run as `./run -r -j 3` on the top level to print `RUN` statuses and launch
    3 jobs in parallel. Note that `subdir1` still runs sequentially (for up to
    3 seconds), making the overall execution last 3 seconds.

### Part 2: Tests as runners

Reported results do not have to follow hierarchy! They can be anything!

* `10-equivalency`
  * This demonstrates that PTEF has no concept of "subrunners" vs "tests", it's
    all just executables in CWD or in subdirectories.
  * Here, `hello` and `world` are the same tests as before, just inside
    subdirectories.
    * Useful for tests that need extra files as metadata / cache.
  * On the flip side, `subdir1` is an executable in CWD, reporting results as if
    it were a subrunner in a directory (faking it).
    * Any executable can thus report any number of any results, simulating many
      levels of virtual hierarchy.
* `11-custom-results`
  * Outside of `PASS`, `FAIL` and `RUN`, the PTEF specification doesn't
    prescribe any standard statuses - in fact, it mentions anyone processing
    results should ignore any results with unknown statuses - this means you're
    free to define your own!
  * `1-skip-nonexec` demonstrates how a custom runner might implement extra
    out-of-spec status that reports `SKIP`ped tests
  * `2-keepalive` shows that a runner can spawn a subprocess/thread which keeps
    reporting result lines while its regular children executables run
  * `3-resources` uses result lines to log basic resource usage of its children
    executables, separately from whether they `PASS`ed of `FAIL`ed
    * (These can also be logged to CSV, to syslog, sent over network, etc.,
      completely outside the PTEF reported results.)
* `12-interface`
  * This is a simple showcase of the report/mklog APIs that PTEF provides for
    various programming languages.
  * Useful as a reference when implementing custom runners from scratch, or
    reporting results from inside tests.
  * Do `make` before issuing `./run`.
* `13-wrappers`
  * Similarly, this is a simple example of how to wrap a full runner
    (as provided by the PTEF reference implementation) in various languages,
    adding custom setup/cleanup or other logic around it.
    * (Or extra switches to the runner, ie. >1 parallel jobs by default.)
  * Here, `bash` is symlinked to `cli`, because there is no Bash builtin for
    the runner function itself. This is due to how Bash internally overrides
    libc's `setenv` (and other) functions, and also how messy its ELF symbol
    namespace is (conflicts with the PTEF implementation's use of libc).
  * Do `make` before issuing `./run`.

### Part 3: Complex from simple

Users of PTEF (test suite authors) are free to break the rules!

* `14-waiving-results`
  * This demonstrates one easy way how to "waive" known test `FAIL`s so they
    show up using a non-`FAIL` state (`WAIVE`), allowing one to treat actual
    `FAIL`s more seriously as they would be unexpected failures.
    * (See the top-level `run` for how this works, and `utils/waive-results`.)
  * Useful for situations where the system under test might not be in full
    control of the tester, and tests might find bugs which take weeks/months
    to fix.
  * Note that the waiving works in real-time and (in this simple example) only
    when run from the top level.
* `15-nonstandard`
  * This provides a few more special cases which don't strictly follow the PTEF
    execution logic (no files/directories to execute), but, on the outside,
    behave like PTEF runners (similar idea to "duck typing").
    * (Passing arguments runs only specific test(s), etc.)
  * Do `make` before issuing `./run`.
    * Before you do, make sure you have
      [LTP dependencies](https://github.com/linux-test-project/ltp/blob/20210524/INSTALL#L1-L18)
      installed.
  * `custom-exit` illustrates how to use the reference runner implementation
    to report custom result statuses based on exit codes of executables
    * The specification only allows 0 = PASS, any other = FAIL, but using other
      states based on exit code is widespread amongst test suites, hence this
      functionality.
  * `etc-scan` scans `/etc` for executable files and reports them as `FAIL`,
    while reporting any non-executable ones as `PASS`.
    * Useful if you want to keep long-term track of which files appeared or
      disappeared over time.
  * `json-checker` reads `*.json` files in its directory and interprets them
    using a custom limited language for checking OS properties.
    * This is similar to
      [OVAL](https://oval.mitre.org/language/about/overview.html),
      a XML-based format for evaluating software vulnerabilities.
  * `syscalls` runs simple per-syscall tests and see if they return `-1`.
    * You should probably use per-syscall `.c` files, not all of them in
      a single process, but this was a nice demonstration anyway.
  * `ltp` wraps a `runltp` script, part of the Linux Test Project test suite,
    which is downloaded and built via `make`, transcribing the specialized
    `runltp` output into PTEF-style results, and providing a (PTEF) interface
    for running single individual tests
    * Since some of the LTP syscall tests take a while, you might wan to run
      with `PTEF_RUN=1 ./run` to get `RUN` results.
    * This test also shows off interactive operation, which is possible with
      stdin on a console, asking user whether they really want to run the LTP
      suite, which may modify the system it's running on.

## Additional examples

See the [contrib/](contrib) directory for even more (free-form) examples,
typically simplified forms of scripts from real-world use.
