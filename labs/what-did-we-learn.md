<p align="center">
<img src="lab-memes/140e-is-pizza.jpg" width="400" />
</p>

## What we learned 

Lots of stuff.   Stuff that gives power and few know.

--------------------------------------------------------
### The Raw Material

20k lines of .md files:
```
    ~/class/cs140e-25win % find . -name "*.md" | xargs wc
    20103 125643 828477 total
```


Almost 40k lines of code (".h", ".c", ".S"):
```
    ~/class/cs140e-25win/labs % find . -name "*.[chS]" | xargs wc
    39830  163894 1266670 total
```

--------------------------------------------------------
The general class approach:
  - real hardware, not simulator.
  - from raw data sheets and arch manual, not pre-digested
    textbooks.
  - deep checking methods to find bugs.  want to be
    surprised if code breaks.
  - each lab = new interesting artifact.

write small example for each concept.  while
small:
 - but they do work,
 - and they run on raw hardware.
 - and you wrote the crucial pieces
 - and you can build out until real.
 - "Show me an example and I'll show you the law"


--------------------------------------------------------
### Data sheets

<p align="center">
<img src="lab-memes/why-read-datasheets.jpg" width="400" />
</p>

Low-level data sheets:
 - GPIO
 - UART
 - machine exceptions
 - Device interrupts
 - Debug hardware
 - NRF (70+ pages)
 - Virtual memory (100+ pages)
 - Crazy VM coherence rules (I've never
   seen a more IQ intensive 12 pages)
 - the next one you read: NBD.

--------------------------------------------------------
### Concepts

Execution is main gerund of Turing world, so we exhausted all angles:

 - cooperative thread
 - pre-emptive thread
 - interrupt
 - exception
 - user process
 - kernel
 - single step 

Assembly:
 - context switching (coop, premptive)
 - vm / tlb / invalidation
 - debug hardware assembly
 - interrupt state
 - lots of you went free range and wrote tons of
   assembly for all sorts of things.

Simple versions of main OS nouns:
 - processes
 - vm
 - file system 
 - networking
 - threads
 - these are all small
   - but they do work,
   - and they run on raw hardware.
   - and you wrote the crucial pieces
   - and you can build out until real.
   - "Show me an example and I'll show you the law"

--------------------------------------------------------
### Checking

We did alot of checking to show the code wasn't bullshit.
Including tricks that no one else knows.
 - cross checking memory state:
    - grab all loads and stores and hash to compare.
    - "show me your tape, and I'll show you your turing machine"
 - use of simulation (fake-pi) to push code through
   paths hard on real hw.
 - use of linker tricks to monitor loads and stores.
 - use of single step to monitor instructions.

EQ checking:
 - verify that not even a single bit changed in even a single register for
   even a single instruction in code that should not be effected.

   This is exceptionally powerful.  Makes test
   cases for free, and checks deeply.
 - EQ check for race conditions (use single step to switch
   every instruction)
 - EQ check for OS: use SS to switch every process on
   every instrution, get same with no VM, with pinned VM,
   with PT vm, with caching, with few process with many
   process, with optimized code, w unopt, etc.


Our belief: These methods can be used to achieve a point where you are
suprised if something is broken.

### How to code

<p align="center">
<img src="lab-memes/knuth-beat-everyone.jpg" width="400" />
</p>

Even if you don't write another line of OS code, hopefully the lack of
safety net + hard bug ingrained a much better approach to writing code.
  - NEVER: write 1000 lines, run it, and use the stare method
    "it doesn't work, why?" 
  - ALWAYS:
    1. working system (verify its working even if you know
      for sure --- easy for something in env to change)
    2. tiny change
    3. validate working.
    4. if not easy to solve.
    5. if so, go back to (2).

Generalized: The Epsilon Paradox:
  - "the shorter your step, the faster you can sprint"
  - working system + one line change = easy linear eq solve
    if wrong
  - working system + X, Y, Z change = hard, multiplicative
    possible.

Common mistake: wrote something, ran it, declare
"it works".
  - No.  It worked once, with a single test.  

  - Maybe lucky with timing;
  - or that you had only a few processes;
  - or that it had internal corruption you didn't check;
  - or cache didn't have conflict or alias;
  - or a missing cache invalidation didn't matter b/c 
    that run didn't load the entry;
  - or that code straddled a 16 byte boundary
    so you got lucky with a pipeline stall to hide a timing
    race with the device;  or the compiler didn't coalesce
  - or swap two loads and stores that it was allowed to;
  - or ...

  - Saying something works if it passed a few test cases is on the level
    of "it compiled, why did it crash"


My belief:
  - yes 10x programmers exist.
  - but it's not necessarily that they are 10x smarter.
  - multiplication of two things:
      1. understand N lines of code better (IQ)
      2. but: able to make delta N smaller (practice)
      3. so: multiply (1) and (2) = 10x
  - IQ is at best fixed, but you can absolutely get better 
    at smaller steps.




### How to debug

Differential (Ockam) debugging:
  - ok you didn't listen and have a broken system.
  - spend your time reverting back to original til
    you have a tiny difference.
  - we gave staff .o's but you can always save yesterdays
    working .o's.

Differential debugging when nothing changed:
  - same: swap out different pieces to isolate.
  - different pi, different laptop, different 
    person doing the checks.

  - In general, probably sw, but 1 out of 20 isn't.


--------------------------------------------------------
### Thanks for great class!

<p align="center">
<img src="lab-memes/fetchquest.jpg" width="400" />
</p>

Largest we've ever had.  Everyone worked incredibly hard.  Lots of crazy
results and hardcore projects.  Learning goal saturation.

And thanks to the staff for often staying til 2am to 
help people:
  - Joe: head TA over and above 
  - Ammar, 
  - Arjun, 
  - Joseph, 
  - Matthew.

Crazy workload.  Crazy class :)

<p align="center">
<img src="lab-memes/let-there-be-exist.jpg" width="400" />
</p>


    ~/class/cs140e-25win/labs % find . -name "*.[chS]" | grep -v staff | grep -v old | grep code | xargs wc


---------------------------------------------------
