# Simulacrum (formerly TheSurrealWaffle)

A basic player automation hack for the Halo PC (retail) and Halo CE (Custom Edition) clients.

[![16 Halo](https://img.youtube.com/vi/16omQqFGpJ8/0.jpg)](https://www.youtube.com/watch?v=16omQqFGpJ8)

## Included Projects

This repository includes:
 * `sentinel`, a framework for developing on the PC/CE clients;
 * `sentutil`, a more approachable interface to the `sentinel` facilities;
 * `sigscan`, a utility library for performing signature scans on executable memory;
 * `detours` (not to be confused with the Microsoft library), which adds patch actions to the signatures of `sigscan`;
 * `example_project`, a basic project that makes use of the `sentutil` library;
 * `debug_utils`, implements some useful console commands for probing the Halo client;
 * `simulacrum` itself.

Most of these projects are not directly related to `simulacrum` itself, but are included as part of `sentinel` and `sentutil`.
When I am sufficiently satisfied with the state of the `sentinel` framework, I will create a separate repository for them.

## Prerequisites

To build, you will need:
 * Boost 1.72 or later (built against), specifically the Boost.Geometry and Boost.Iterator libraries;
 * GCC 9.2 or greater, capable of compiling 32-bit applications (built against);
 * CodeBlocks, to open and build the projects.

To build with CodeBlocks, open the workspace file (`sentinel.workspace`) and Build Workspace.
You will likely need to change the compiler the project files use.
`sentinel.dll` will be copied into the `build` directory.
The `sentinel` modules included in this project will be copeid into `build\modules`.

## Installing

 1. Copy `sentinel.dll` into the `controls` (or `mods` if you are using the beta release of Chimera) subfolder located within the Halo install directory.
 2. Create a folder named `sentinel` in the Halo install directory.
 3. Copy `simulacrum.dll` into the `sentinel` subfolder.

Alternatively, one may add the `build\modules` directory to the environment under the variable `SENTINEL_MODULES_DIRECTORY` and ignore steps 2 and 3.

## Additional Features

If `sentinel` loads successfully into Halo, then the following features are added:
 * command-line option `-no-sentinel`, which effectively turns off `sentinel` entirely;
 * command-line option `-create-console`, which allocates a separate console that presents debug information;
 * a fix for console text fade speed at high framerates.

## Forewarning

This project spans a slow decade of development and various compilers, standards, and preferences.
As a result, quality and style are likely to vary throughout and through time as I update this project.
For instance, the `detours` and `sigscan` libraries have a lot wrong with them that I hope to correct, granted the time to focus on them rather than `simulacrum`.
Stylistically, there is much within `sentinel` that I wish to change and add.

## License

The projects listed above are licensed under the Boost Software License Version 1.0, see [LICENSE_1_0.txt](../blob/master/LICENSE_1_0.txt) for more information.
