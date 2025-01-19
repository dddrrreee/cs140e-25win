### Some simple measurements.

Being able to measure time lets us measure different things, so we do so:
  - `0-cycles-per-sec.c` shows how to measure how many cycles per 
    second the pi zeros we gave out.  The standard pi runs at 700Mhz
    when no over-/under-clocked, but the blurb on the digikey listing
    that we bought from claimed a 1GHz CPU, so I'm curious what everyone
    gets.

Time also cuts across all abstraction barriers which lets us figure out 
interesting things, so we do that to:

  - `measure.c` shows how much different things cost using the cycle
    counter.  It also shows a weird timing issue that comes up 
    with alignment.


Also has a simple example of getting and printing the mode:
  - `mode.c`
