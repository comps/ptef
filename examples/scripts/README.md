# Scripts example

This showcases a more controlled execution environment, with conditional
logic and different ideas of what to run by default. Not all levels need
this and can thus use symlinks (to the original runner binary or any of
the generic "just run all" scripts).

To run this, run `make` in top level of the repository (to build the runner)
and then ensure that it is available in PATH, ie. by
```
export PATH="$PATH:$PWD/bin"
```
in the top level of the repository. Afterwards, `./run` in any of the
sub-directories.
