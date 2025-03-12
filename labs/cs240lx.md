## CS240lx-25 pitch

<p align="center">
<img src="lab-memes/chaotic-study.jpg" width="400" />
</p>

If you liked 140e, consider taking cs240lx next quarter. 
  - it's fun.
  - it's Joe's last quarter before graduation.
  - we have record enrollment so by the birthday paradox 
    / pigeon hole principle, your fav people will be there.


Same rough format: 
  - still 2 days a week.
  - still hardware, low level code.
  - still pizza.

Differences:
  - usually stop around 1030 or so, vs 1230.
  - is more conceptual based.
      - 140e must cover execution, vm, fs, some network.
      - 240lx assumes you know that stuff, and use it to do
        other stuff.
  - class can replace cs240 (paper reading).
  - usually has the people who find 140e fun (+/-).
  - the final projects are wild.

Two old, partially overlapping offerings:
  - [cs240lx-22](https://github.com/dddrrreee/cs240lx-22spr/tree/main/labs)
  - [cs240lx-23](https://github.com/dddrrreee/cs240lx-23spr/tree/main/labs)


Topics:
  - cool tricks i picked up over 3+ decades. or seem not well covered.
  - depends alot on what people are interested in.  can be
    OS heavy, or more device heavy, or more project heavy
    depending.  let us know what kind of stuff you are into.
  - Probably at least one new board (pico, different riscv).


Always generate executable code at runtime
  - even more low level than inline assembly.
  - self-modifying code is the ultimate sleazy hack.
  - used to make fast(er) interpreted languages (JIT for
    javascript, etc), fast dynamic intstrumentation (valgrind),
    and old school virtual machines (vmware).
  - can do all sorts of speed hacks by specializing using runtime
    information.  compiling data structures to code.  etc.

Always do a custom pcb:
  - parthiv historically comes in to do a week of custom pcb labs so
    can make your own cool boards.


Always build a bunch of tools using memory faults and single-stepping.
  - Eraser race detector 
  - Purify memory checker 
  - Advanced interleave checker.  
  - instruction level profiler measuring cycles, cache misses,
    etc.

If we have more kernel hackers some subset of:
  - network bootloader
  - make an actual OS that ties all the stuff together.
  - small distributed system
  - distributed file system
  - actual clean fat32 r/w so can do distributed file system,
    firmware updates.
  - processes that migrate from one pi to another.
  - FUSE file system interface.

Always do some speed:
  - make low level operations (exceptions, fork/wait, pipe
    ping pong) 10-100x faster than laptop.  i think this is
    feasible but haven't done, so am interested.
  - overclocking the pi and seeing how fast can push it.
  - push the NRF using interrupts and concurrent tx, rx to 
    see how close can get to the hw limit.

  - make a digital analyzer (printk for electricity) where
    you get the error rate down to a small number of nanoseconds

And a few different communication protocols: 
  - how fast can send/recv data over gpio pins b/n two pis?
  - over IR
  - over speaker/mic.
  - over light and camera (?)
  - lora?


Device labs are fun.  So we do those too:
  - accelerometer, gyro
  - lidar
  - addressable light array
  - analog to digital converter.
  - lora so can send bytes 1km+

More systems-y device labs, by combining into
standalone tools:
  - acoustically reactive light display using mic, adc, 
    addressable light array.  extend to multiple systems.
  - little osscilliscope using oled display, mic, adc

Lots of other stuf.
  - e.g., maybe some other languages (rust?  zig?)
  - static bug finder.



---------------------------------------------------------------------------
#### What we need

If you have time:
  - let us know which topics you're more into.
  - whether 6pm in same room as 140e is better
    than 530 (or 430?) in different room.

Also: debating doing two new classes:
  - device class were we do the fun stuff.
  - low-level tools class (replacement for cs343 
    advanced compilers) where we build a bunch of
    tools --- static, dynamic, simulator, symbolic.

---------------------------------------------------------------------------
<p align="center">
<img src="lab-memes/aspirations.jpg" width="400" />
</p>

