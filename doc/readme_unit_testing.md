# Unit testing guide

This unit testing guide compiles for the a specific architecture to be run on the same docker container.

## Prerequisites
 - [Check Out Submodules](./readme_building.md#check-out-submodules)
 - [Build the Docker Container](./readme_building.md#build-the-docker-container-arm64-compilation)
 - [Activate Matter development environment](./readme_building.md#activate-matter-development-environment)
 

## Compiling the unit test

Unit test can be compiled using the below steps:

```bash
root@docker:/unify-matter-bridge$ cd /unify-matter-bridge/linux
root@docker:/unify-matter-bridge/linux$ export LD_LIBRARY_PATH=/unify-matter-bridge/linux/out/<arch>_test/gen/stage/lib
root@docker:/unify-matter-bridge/linux$ ../scripts/compile_tests.sh -t <arch>
```

This generates test binaries inside `out/<arch>_test/tests` where test executables can
be found.

## Running the unit test

Unit tests are run by first locating your test binary and then executing it.

```bash
root@docker:/unify-matter-bridge/linux$ ../scripts/run_tests.sh -b out/<arch>_test
```

Unit tests can also be executed individually by using the test binary.

```bash
root@docker:/unify-matter-bridge/linux$ ./out/<arch>_test/tests/TestExample
'#0:','ExampleTests'
'#3:','Example::TestExample','PASSED'
'#6:','0','1'
'#7:','0','1'
```

## Getting unit test coverage

To understand unit test coverage, run the following helper script:

```bash
root@docker:/unify-matter-bridge/linux$ ../scripts/get_test_coverage.sh -t <arch>
```

## Adding new unit test cases.

The Matter bridge test infrastructure leverages the test ecosystem of Matter itself. Writing a unit test is done with the NL Unit test framework and Matter helper functions. Please refer to [Test example](../linux/src/tests/TestExample.cpp) for an example of how to write a simple unit test. After writing a unit test remember to add the source file in the [BUILD.gn](../linux/src/tests/BUILD.gn).

```{toctree}
---
maxdepth: 1
titlesonly:
hidden:
---
readme_clustertest.md
```

Refer [Cluster Unit Test Case](readme_clustertest.md) for more details on how to write unit test to test cluster support added to matter bridge.
