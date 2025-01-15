## 5-safety

[This is the final version of the makefile](./Makefile).  
You should be able to use this for most situations without much fussing.
It 
  - uses wildcards to get all the source dependencies.
  - automatically generates the dependencies
  - ... the include flags to pass to the compiler.
  - disables built-in make rules so we don't get
    suprised and explicitly lists out the files each rule
    applies to for the same reason.

It has a bunch of extra checking and safety steps that are useful but
not required --- you can delete them if you want it to be shorter.
