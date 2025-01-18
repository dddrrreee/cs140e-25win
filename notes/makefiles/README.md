<p align="center">
  <img src="./images/build-system.jpg" width="600" />
</p>


### tl;dr

This directory goes over possibly the only two `Makefiles` you will need
in your career, a simple one and a fancy one, and why they are written
the way they are.  We also give a third variant that does things with
less machinery than the fancy version.

 1. [basic-makefile-unix](./basic-makefile-unix) which works through
    everything in a simple way.  The final `Makefile` should work for
    most of your projects.

 2. [fancy-makefile-unix](./fancy-makefile-unix): a fancier
    version that is likely the best robust approach for large projects
    through its use of the `eval` function (an extension of `GNUmake`)
    that can be used to generate explicit compilation rules.  Tweaks of
    this `Makefile` should work for any project you do, afaik.

 3. [less-fancy-makefile-unix](./less-fancy-makefile-unix):
    this version doesn't use `eval` but instead achieves a *mostly*
    similar result by making the build directory structure mirror the
    source directory so that it is easy to make a bijection between the
    source and `.o` and `.d` files.  It has additional error checking
    and comments.

    It's good to go through this version even if the `eval` version
    works for you since it talks about things in a slightly different way.

All of them are written to guard against or eliminate the common mistakes
described below.

Usefufl reading:
  - [thorough, concrete makefile tutorial](https://makefiletutorial.com/).

  - [make wikipedia page](https://en.wikipedia.org/wiki/Make_(software)).
  - [simple concrete makefiles](http://nuclear.mutantstargoat.com/articles/make/).
    While the domain name is weird, the `make` examples are concrete,
    simple, useful.
  - [short makefile tutorial](https://www.lucavall.in/blog/crafting-clean-maintainable-understandable-makefile-for-c-project)

  - [Auto-dependency generation](https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/).  Weird site name (a theme!), but the 
    writer is or was the GNU `make` maintainer, so contents are high signal.

Suggestions:
  - [O'Reilly make book](https://www.oreilly.com/openbook/make3/book/)

### Overview

As projects get large, manually compiling code is a pain.   The Unix
`make` program is one of the oldest ways to automate this process.
`make` has a deserved reputation as an unusually ugly and baroque
input language.  However, it exists everywhere, is used many places,
and has enough power embedded inside that you can often make it do what
you want --- compiling based on what changes with automatic dependencies,
producing multiple programs or libraries and placing them in
different locations, automatically running regression tests, etc.


We use `make` alot.  Many places you might work or found will as well.
Despite its baroque weirdness, there is a narrow  slice that will do
most of what you need.  We cover much of that slice today by doing
increasingly fancy versions of a `Makefile` that does two things:

  1. Produces an executable file `main` by compiling the C source
     files `a.c`, `b.c`, `c.c`, `main.c`.

     Our makefile structure is driven by two correctness rules.

     Correctness rule 1: any source file must be recompiled if
     it changes.  The `main` must also be re-generated.
  
     Correctness rule 2: All of the example files `#include` a header file
     `header.h` and so must be recompiled if `header.h` changes.

     In general, a source file must be recompiled if anything it depends
     on changes.  For today we only worry about the header `header.h`.
     Of course, the makefile itself is a dependency.  

     In the limit, the operating system installation, compiler
     (assembler, linker, preprocessor) or libc implementations can be
     seen as dependencies as well ---- in some cases you may want to
     recompile if any of these changes.  We do not do this today.

  2. After producing the `main` executable, check that running `main`
     gives the same result as a reference output `main.ref`.

--------------------------------------------------------------------
### Common `make` mistakes

Below is an incomplete list of common `make` mistakes (all have burned us)
and our current view of how to mitigate / solve them.  Full solutions
are in the worked-out example `Makefiles`.    Note, some of the 
problems below are low-level enough they might not make sense without
working through some of the `make` examples given above.

Before getting into the concrete low level mistakes, the largest, most
common general mistake I see is that because `make` is a weird language,
people seem to forget all the methods they learned for writing correct
code / debugging programs in all the other languages they know.  This is
especially a problem b/c `make` doesn't have a debugger.

 1. They write stuff all-at-once then try to get it to work.
    You should do it just like you do kernel code.  Write the smallest
    thing possible, test it, then do the smallest step, test it, etc.
    The smaller the step, the more obvious the bug.

    Also, and not unrelated: test it!  `touch` each file and make sure it
    gets remade.  Delete each result (.o, .d) and make sure the program
    gets remade.  

    Related: If you have to do `make clean` before every `make`, fix
    the damn bug.

 2. A second consequence of it being a weird language is that people
    get unsually passive when trying to debug.  Most common approach:
    the stare method, where they look at their screen without moving, 
    with mixed success.

    Instead, be active!  Make a copy of the `Makefile` (or entire build
    tree) and try to cut it down to the smallest broken version.  Put in
    print statements!  For different rules, to print variables, etc.

    You can also run `make -d` or `make --debug` but their outputs are
    not always intuitive.

    Common: if you forget what the many built-in `make` variables
    mean, just print them. 

 3. Program defensively.  Write rules to guard against common mistakes.
    Write if-statement checks in `make` and give errors.  For example,
    if you are going to put all output in a directory whose name
    is held in `BUILD_DIR` --- make sure that `BUILD_DIR` is not empty:


        ifeq ($(BUILD_DIR),.)
            $(error "<BUILD_DIR> cannot be '.' or clean will remove everything.")
        endif

    We give other examples below.

More concrete mistakes:

  - Mispell `make` variables.  You won't get an error, instead the 
    mispelled variable will just be empty.

    This is especially a problem when the variable is used as a
    compilation rule's dependency since it will be empty, and thus
    doesn't give a dependency.  The first `make` will work correctly
    (since it builds from scratch) as will a `make` after `make clean`,
    but later ones can use stale .o's.

    Solution: be careful (sorry; `make` sucks for this).  You can
    somewhat guard against this with how you write rules (below).
    Also when writing makefiles add the warning for unused variables to
    `make`'s flags:

        MAKEFLAGS += --warn-undefined-variables

  - Give the wrong path when using `include` for automatically
    generated `.d` dependency files.  `make` will silently proceed, and
    you have no dependencies.  Again: will work the first time or after
    `make clean` but will allow stale results later.

    Solution: add each `.d` as an explicit dependency for each `.o` rule.
    If the `.d` doesn't exist, will get an error.

        $(BUILD_DIR)/%.o: %.c $(BUILD_DIR)/%.d  $(DEPS)
            $(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

  - `make` automatic rules do the wrong thing (e.g., avoid the 
    `.d` safeguard above).

    Solution: disable built-in rules either individually or
    (preferred) en masse:
    
        # disable all built-in rules.
        .SUFFIXES:

    This gives you complete control over the build.

  - Change the `Makefile` but forget to `make clean; make`.
    Often fine, sometimes bad.  

    Solution: add a dependency on the `Makefile` itself.  You can pull 
    out the `Makefile` and all the other names as follows:

        # pull out every Makefile from the confusingly-named
        # <MAKEFILE_LIST> (which can contain .d files if they
        # get included)
        MK_LIST := $(filter-out %.d, $(MAKEFILE_LIST))

    Or do the hack of automatically running `make clean` when the
    `Makefile` is newer than a timestamp file.  (We have examples
    of both.)


  - A file appears more than once in the list of directories `VPATH`
    contains.  First resolution will win, no matter if you intended to
    grab a later one.

    Solution: don't use `VPATH`!  Several alternatives:
      1. Explicitly generate the rules using `eval`.
      2. Have a build directory that mirrors the source directory.
      3. Generate the `.o` in the same directory as each source file 
         (ugly, but simple).  

      Generally all three mean that duplicate files that contain the
      same routine will show up with multiple definitions.

  - `make` is deleting intermediate files (e.g., `.list` files)

    Wrong solution: add a `.PRECIOUS` directive:

        .PRECIOUS: %.list %.elf 

    This will leave garbage if you control-c during `make`.

    Correct solution: explicitly list out the files that are produced
    by rules.  This also partially checks that your internal rules
    work correctly.

  - The build directory you want to put intermediate results in doesn't
    get created robustly.

    I used to use a target for the build dir, but this is really finicky
    for reasons I actually don't understand.  

        # <FORCE> forces the rule to run always.
        $(BUILD_DIR): FORCE
			mkdir -p $(BUILD_DIR)
    
        FORCE:
        # tell make FORCE doesn't generate a result.
        .PHONY: FORCE

    Solution: just put the `mkdir -p $(BUILD_DIR)` with each rule.
    This is kind of ugly, but the way we do things there aren't many
    locations and it is guaranteed to always work.  E.g., something
    like:

        $(BUILD_DIR)/%.o: %.c $(BUILD_DIR)/%.d  $(DEPS)
            @mkdir -p $(BUILD_DIR)
            $(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

### Summary

While the there are a long list of ways that `make` will do what you
tell it but not what you want, the fortunate thing is that you likely
only small variations of a couple `Makefiles` in your life and we give
them as examples.
