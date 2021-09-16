# Portable Test Execution Framework (PTEF)

A simple (KISS) specification and an example implementation of a test "runner"
framework for system testing and/or integration and execution of test suites.

[Read the specification here!](src/doc/ptef.adoc)

The "portable" refers to its inspiration from POSIX concepts and C API - the
specification can be implemented purely using POSIX.1-2008 and the reference
implementation does so (no GNU extensions).

## The deal

The specification above is pretty simple, but this is the basic idea:

* Have a directory tree of executable files (and other unrelated data)
* Run everything in CWD
  * If a file is executable, just run it
  * If it's a directory, see if there's an executable file inside it
    called after `basename(argv[0])` of the current process
    * If so, `cd` into it and run the file there

This allows for natural execution flow propagation throughout a hierarchy.

Result reporting is then (and this feels revolutionary) done independently
of this hierarchy - it should follow it, but any executable can basically
report anything.

```
PASS  /some/dir/blabla
FAIL  /some/dir/extra-test
PASS  /some/dir/multi-test/x/y/z
```

Here, `some` and `dir` are directories, `blabla` and `extra-test` are plain
executables - their statuses are reported by the parent logic running them
(see above).  
On the other hand, `multi-test` is an executable which hijacks the system and
reports custom results for a "virtual" hierarchy. This is essential for wrapping
other test suites and transcribing their results, or even for just reporting
as status for every file in `/usr` that passes/fails some test.

The reference implementation (under [`src/`](src)) provides command-line
utilities, native C functions, Python bindings and Bash builtins, all using
the single ABI of `libptef.so` (accessible to other languages).

There's much more and most of it is using neat basic \*nix concepts, go see
[the full specification](src/doc/ptef.adoc) or dive into annotated
[examples](examples).

## How to install

### Fedora

Pre-built RPM packages are currently available via Fedora COPR (remove `-y`
to make it interactive).

```sh
dnf -y copr enable jjaburek/ptef
dnf -y install ptef
```

### Building from source

Note that the `make install` here needs to be done as `root` (ie. via `sudo`)
as it will install everything onto your system.

```sh
# install build dependencies (adjust for your distribution)
# - asciidoctor is optional - if missing, you won't get a nice HTML version
#   of the PTEF spec in /usr/share/doc
dnf -y install gcc make python3 bash bash-devel asciidoctor

# build (use gmake on non-Linux)
make

# run test suite (optional)
dnf -y install valgrind
make test

# install (into /usr)
make install
```

The build system respects
["standard" directory variables](https://www.gnu.org/prep/standards/html_node/Directory-Variables.html)
so you can install in `/usr/local` as `make install prefix=/usr/local`. It also
honors `DESTDIR`, allowing you to set a new root for the installation, ie.
`make install DESTDIR=$tmpdir` which works even unprivileged if `$tmpdir` is
owned by the unprivileged user.

This should really be done only in (temporary) containers, ie. in various
CI systems, as mixing `make install`-ed binaries with RPM/DEB installs can
cause issues on persistent systems.

## License

Unless otherwise specified (such as at a beginning of a file), all content in
this repository is provided under the MIT license.  
See the [LICENSE](LICENSE) file for details.
