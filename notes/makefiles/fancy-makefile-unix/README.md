## Semi-fancy `Makefile` for a large project.

Current best way I know to: 
  1. take a bunch of source files spread across multiple directories 
     (including directories with relative paths above the build
     directory).
  2. Have complete control over the rules for generating .o and .d 
     (through the use of eval): no vpath, no implicit rules, zero
     abiguity possible about which .c file is used for which .o file.
  3. put the .o's in a build dir.
  4. put the .d's in a build dir.

Why:
  1. vpath will eventually cause hard-to-debug problems if the same file
     name appears in different directories.  ***Even if this name is
     not explicitly included in your source list!***   

     Problem:
       1. Your makefile initially uses `bar/bar.c`.  Let's say
          your VPATH is the trivial:

                VPATH = bar/

       2. You want to add `foo/foo.c`.
       3. So you add add `foo/` to your vpath before `bar/`:
            
            VPATH = foo/ bar/

       4. This is fine in general, so you get used to doing this.
          Very easy!

       5. But one day you do so for a `foo/` that just happens to 
          contain a `bar.c` (that you don't intend to use).
       5. `make` will silently use `foo/bar.c` instead of `bar/bar.c`.
       6.  If you reverse the vpath, the same problem happens
           if `bar/foo.c` exists.

       7. This situation is exceptionally bad if the faux amime
          that gets silently swapped in behaves *almost* identically to
          the original.  (This burned us very badly for the UART lab
          with `cstart.c`.)

     The problem is analogous to adding a directory `foo/` early to
     your shell PATH variable so your shell can locate a program `foo/`
     contains, but b/c `foo/` contains other executables as well, the
     shell will pick these as well, rather than getting the intended
     executable for a directory that appears later.  It's a fundmamental
     danger of having a flat list of contexts where the first match wins.

  2. the hack of making the build directory mirror the source
     directory (as we do in `full-makefile`) is a pain when grabbing
     source files using relative paths that go above the `Makefile`
     directory (e.g., as they do if we include lab code into `libpi`).

     you can convert to relative paths to absolute, but then you get
     these crazy long directory trees in your build dir.  there does not
     seem to be a nice way prune off the largest common directory prefix.
     
     to some degree this is an aesthetic concern, but it makes
     compilation lines enormous, which makes debugging hard, so is
     sort-of a correctness issue.

  3. the foundational problem is that afaik `make` does not let you map
     from a source file to its .o.  `make` rules require that you can
     go from .o to .c.  if you put the .o in a build dir it's unclear
     mechanically how to derive the source directory from multiple
     options.

  4. we use explicit rules to eliminate all sorts of problems where built-in
     rules allow `make` to falsely think it knows what to do.

AFAIK the basic `Makefile` in this directory is correct and has a good
defensive approach to all sorts of mistakes.  let me know if you find
any issues!
