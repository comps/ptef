# Portable Test Execution Framework (PTEF)

A simple (KISS) specification and an example implementation of a test "runner"
framework for system testing and/or integration and execution of test suites.

* [6-minute video "sales pitch"](https://www.youtube.com/watch?v=FodeosLO_RM)
* [PTEF specification](src/doc/ptef.adoc)

The "portable" refers to its inspiration from POSIX concepts and C API - the
specification can be implemented purely using POSIX.1-2008 and the reference
implementation does so (no GNU extensions).

## The deal

The specification above is pretty simple, but this is the basic idea:

* Have a directory tree of executable files (and other unrelated data)
* Process files in CWD
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

There's much more and most of it is using neat basic \*nix concepts,

* go see [the full specification](src/doc/ptef.adoc)
* dive into annotated [examples](examples)
* [watch the examples as a 70-minute video](https://www.youtube.com/watch?v=qMbxrhlguhk)
  (instead of running them yourself)

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
dnf -y install diffutils valgrind
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

## API Stability

There are currently no guarantees that the C-based function API, bash and python
APIs won't change in the future. There's also no plan in place for keeping API
stable within a versioned release, though that may come if this project becomes
popular.

For now, expect having to update your test suite's PTEF-interfacing code when
moving to a newer PTEF version.

## ABI Stability

Despite `libptef.so` being an ELF shared object that *could* have ABI made from
versioned symbols, maintaining this over time is outside the scope of this
project and would just be a PITA with very few benefits.

You're expected to maintain tests in a source code form and build them
immediately prior to running them, not have a collection of compiled binary
tests that last years and magically work with future PTEF versions.

## Portability

The PTEF specification itself can be implemented in virtually any programming
language on any platform or libc which provides access to POSIX.1-2008 C API,
namely the functions mentioned in the specification.

* The reference implementation is mostly-C99 with `_POSIX_C_SOURCE=200809`
  extensions.
* The python3 interface to the reference implementation currently requires
  python 3.6 or newer.
* The bash interface (bash builtins, not standalone CLI utilities) requires
  bash 4.4 or newer.

Note that the build process is smart enough to detect the lack of `python3`
in PATH, or the lack of bash development headers, and avoids building those
interfaces.

The build system used for the reference implementation is GNU Make (using
several of GNU-specific extensions) - this should be fairly widely available
as `gmake` on \*nixes.  
The build system also uses `/bin/bash` to avoid the myriad of differences
between POSIX-like shells, though any 3.x or newer version should work just
fine.

Feel free to report an issue (or send a pull request) on the build system
if you run into issues. It's possible that ie. the C compiler options are
not compatible with \*BSD linkers.
