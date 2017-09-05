# ali - advanced Lua integration

This project is a reference design for a set of libraries supporting
advanced integration of C++ with Lua.   The libraries are:

- aliSystem
- aliLuaCore
- aliLuaExt
- aliLuaExt2
- aliLuaExt3

The aliSystem library provides some abstractions to general application
utilities such as threading, serializing, time, process initialization, and
a handful of other interfaces.

The other aliLua* libraries provide various interface related to integrating
Lua with C++.  The aliLuaCore and aliLuaExt provide a basic environment.
aliLuaCore provides the basic integration features, though it does not
provide a concrete implementation of aliLuaCore::Exec.  aliLuaExt provides
aliLuaExt::ExecEngine and aliLuaExt::ExecPool, which provide an implementation
of the Lua execution interface that would support the features exposed by
the aliLuaCore library.

This project was designed to be an educational reference design rather than a
production library optimized for some more specific purpose.  Despite being
designed as an educational reference library, it offers a solid base for many
production challenges.

The dependencies between the libraries are as follows:

- aliSystem  - none
- aliLuaCore - aliSystem
- aliLuaExt  - aliSystem, aliLuaCore
- aliLuaExt2 - aliSystem, aliLuaCore
- aliLuaExt3 - aliSystem, aliLuaCore, aliLuaExt

There is a single application directory app, which is more of a trivial
example of linking with the ali* libraries.  Additional example projects
will likely evolve in the future.

# Testing

There are two test suites:

- aliSystemTest
- aliLuaTest

Their build products are:

- aliSystemTests
- aliLuaTests

Their dependencies are:

aliSystemTest - aliSystem
aliLuaTest    - aliSystem aliLuaCore aliLuaExt aliLuaExt2 aliLuaExt3

aliLuaTest is includes tests to cover all of the aliLua* libraries.
Although this suite could be split into a test suite for each of the
libraries, at this time it is not.

# Building

The build is defined with cmake.  To build the library, the sample
application, documentation, and to run the tests, one can just:

```
mkdir build
cd build
cmake ..
make
aliSystemTest/aliSystemTests
aliLuaTest/aliLuaTests
```

# Development

The libraries are built with C++14.

Original development was done with Ubuntu 16.04 on an i7-4750HQ processor
with gcc version 5.4.0 20160609.

The original work was done with googletest verion 1.8.0+ (it was ~12 verions
past the tagged 1.8.0).

At this time, the project is unlikely to build on a windows machine outside
of some linux emulator such as cygwin, though even that has not been
attempted.  Though the test suite should be complete enough to catch many
issues, it may not be thoroughly testing cross platform issues.

Development was done with Lua 5.3.3.  No testing has been done with Lua 5.2
or Lua 5.1 versions to analyze compatibility issues.

