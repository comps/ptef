# Symlinks example

This illustrates a basic usage of a runner than runs all executable files
in a hierarchy of runners (executables in directories without `run` are
ignored due to how the execution logic works - see the spec).

There is no configuration, no additional setup/cleanup done by the runner.

To run this, run `make` in top level of the repository (to build the runner)
and then just execute `./run` on any level inside this directory.
