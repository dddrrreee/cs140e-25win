Various hints:
  - we probably should have called `saved_sp` just `sp` since it is always
    the value of the associated thread's  `sp` register. 

  - make sure you initialize `sp_saved` in `rpi_fork`.

  - the only place that the `stack` field should be referenced is at
    the start of `rpi_fork`.  You should only use `saved_sp` everywhere
    else.

  - if you're having trouble with trampoline: one helpful fact is that 
    `rpi_cswitch` always jumps to whatever is in the `lr` register.
    It loads this from the stack.  So if we can write that offset (which
    should at `LR_OFFSET` as per the `2-write-regs.c` test case) then
    we can make `rpi_cswitch` jump to the trampoline.

Missing:  there are various things we should have done but did not:
  - there should be "redzones" with sentinal values above and below
    the stack so you can verify nothing got corrupted.  similarly,
    initialize the stack to  known value so you can check that nothing
    below the sp got corrupted

  - should have you write stack bounds checking.

  - detect leaks by tracking number of freed/allocated blocks.
    freed should equal allocated when the package exits.  no new
    allocations should happen if you run / exit over and over.

  - maybe for the scheduler thread just have a single 32-bit word for
    the save point?  not sure.

Extensions:
  - maybe the best extension is to figure out how to integrate your system
    with the interrup handler.

  - extension at some point: speed up the circular buffer.  it's crazy slow at
    the moment.
