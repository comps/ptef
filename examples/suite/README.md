# Suite example

This is a more advanced usage example, see the other ones first for easier and
more obvious examples.

This example shows the runner in a non-interactive use case of an automated test
suite executed ie. by a Continuous Integration system, collecting the results
and reporting detailed log output of any failed tests.

It also implements custom runners using tools provided by the reference runner
implementation, simulating a "virtual hierarchy" of a 3rd party test suite,
integrated into this one.

Finally, it combines TEF with other systems like (GNU) Make within the same
hierarchy, in addition to test libraries and sub-hierarchies that don't use
TEF at all.

To run this, run `make` in top level of the repository (to build the runner)
and then ensure that it is available in PATH, ie. by
```
export PATH="$PATH:$PWD/bin"
```
in the top level of the repository.

Afterwards, build this example suite with `make` in this directory (where this
README is) and follow it with `make run`. Note that this works only in the TLD
of the suite, which doesn't support running from any arbitrary hierarchy level.
