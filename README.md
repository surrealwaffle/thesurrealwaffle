# Simulacrum (formerly TheSurrealWaffle)

A basic player automation hack for Halo PC (retail) and Custom Edition.

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

In order to build these projects, you will need:
 * Boost 1.72 or later (tested), specifically the Boost.Geometry and Boost.Iterator libraries,
 * A 32-bit compiler with C++20 support (GCC 9.2 with -std=c++2a on MinGW suffices).

## Installing

 1. Copy `sentinel.dll` into the `controls` subfolder located within the Halo install directory.
 2. Create a folder named `sentinel` in the Halo install directory.
 3. Copy `simulacrum.dll` into the `sentinel` subfolder.
 
## Forewarning

This project spans a slow decade of development, various compilers and standards, changing preferences, and experimentation with new language features.
As a result, quality and style are likely to vary throughout and through time as I update this project.
For instance, the `detours` and `sigscan` libraries have a lot wrong with them that I hope to correct, granted the time to focus on them rather than `simulacrum`.
