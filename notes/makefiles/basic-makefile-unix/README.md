# Simple unix makefile.

The following directories develop a `Makefile` that should work for
most small projects you do in your career.  The final iteration
(`5-safety`) produces a executable by:
  - automatically gathering up all source files in a given 
    set of directories;
  - automatically computes their dependencies so they will get
    correctly remade no matter which file gets modified;
  - automatically computes their include flags;
  - takes steps to guard against common mistakes.

The steps:
  - [0-most-basic](0-most-basic): simplest version that hardcodes
    all files and dependencies and recompiles everything to produce
    the `main` executable.

  - [1-incremental-compilation](1-incremental-compilation): uses
    incremental compilation.
  - [2-pattern-rules](2-pattern-rules): use pattern matching so 
    the rules get more automatic.
  - [3-wildcards](3-wildcards): use wildcards to automatically
    get source code and dependencies.
  - [4-auto-dependency-gen](4-auto-dependency-gen): use the compiler
    to automatically generate dependencies.
  - [5-safety](5-safety): add safety checks and some additional tricks.
    This should be most of what you need for most personal projects.
