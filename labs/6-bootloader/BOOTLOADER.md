## The bootloader protocol.

At a high level, the protocol works as follows: 
  1. pi: spin in a loop, periodically asking for the program to run;
  2. unix: send the program code;
  3. pi: receive the program code, copy it to where it should go in
     memory, and jump to it.
  4. To detect corruption, we use a checksum (a hash) that we can
     compare the received data too.

This is a stripped down version (explained more below):
 - The `boot_put32` calls transmit a 32-bit quantity,
   `boot_get32` receives (`boot_get8/boot_put8` is 8 bits).
 - The pi transmits and receives using its internal UART (the same 
    device used for `printk`: implemented next lab).  
 - The Unix side transmits and receives across an OS managed tty device.
 - If any check fails, the pi side sends the Unix side a `BOOT_ERROR`.


```
     =======================================================
             pi side             |               unix side
     -------------------------------------------------------
              (1)
      boot_put32(GET_PROG_INFO)+ ----->
      [resend every 300ms 
          if no response]
                                            (2)
                                      // discard garbage
                                      while(!<GET_PROG_INFO>)
                                        boot_get8();  // get 8 bits
                                      boot_put32(PUT_PROG_INFO);
                                      boot_put32(ARMBASE);
                                      boot_put32(nbytes);
                             <------- boot_put32(crc32(code));
            (3)
      if(<code collision>)
        boot_put32(BOOT_ERROR)
        reboot()
      boot_put32(GET_CODE)
      boot_put32(crc32)      ------->
                                            (4)
                                      // discard further GET_PROG_INFO
                                      while(<GET_PROG_INFO>)
                                            ;
                                      <check crc = the crc value sent>
                                      boot_put32(PUT_CODE);
                                      foreach b in code
                                           boot_put8(b);
                              <-------
            (5)
     <copy code to addr>
     if(!<check code crc32>)
        boot_put32(BOOT_ERROR)
        reboot()
     boot_put32(BOOT_SUCCESS)
     jump to <ARMBASE>
                              ------->
                                            (6)
                                       <done!>
                                       start echoing any pi output to 
                                       the terminal.
     =======================================================
```

More descriptively:

  1. The pi will repeatedly signal it is ready to receive the program by
     sending `GET_PROG_INFO` requests every 300 milliseconds. 

     (Q1: Why can't the pi simply send a single `GET_PROG_INFO` request?)

  2. When the unix side receives a `GET_PROG_INFO`, it sends back
     `PUT_PROG_INFO` along with three other 32-bit words: the constant
     to load the code at (for the moment, `ARMBASE` which is `0x8000`),
     the size of the code in bytes,  and a CRC (a collision resistant
     checksum) of the code.

     (Q2: Why not skip step 1 and simply have the unix side start first
     with this step?)

     IMPORTANT: the Unix side may have to discard unexpected bytes left in
     the tty until it hits a `GET_PROG_INFO`.  Two possible causes.  One,
     the tty channel could still contain garbage bytes previous run of a
     pi program (e.g., from its `printk` calls).  Two, a race condition!
     This might seem weird since we aren't using threads internally on
     either end, but occurs because pi and the Unix side are running
     concurrently and sharing the tty device.  The Unix side could have
     opened the tty connection in the middle of the pi side sending
     the 32-bit and so only received the last 24, 16 or 8 bits of a
     `GET_PROG_INFO`.

     (Q3: why not use a `get32` instead of a `get8` for this step?)

  3. The pi-side checks the code address and the size, and if OK (i.e.,
     it does not collide with its own currently running code) it sends a
     `GET_CODE` request along with the CRC value it received in step
     2 (so the server can check it).  Otherwise is sends an error
     `BOOT_ERROR` and reboots.

     Since the pi could have sent many `GET_PROG_INFO` requests before
     we serviced it, the Unix code will attempt to drain these out.

     (Q4: why could there be more `GET_PROG_INFO`?)


     (Q5: how to check the code?  How to prevent overflow in
     the size?)

  4. The unix side sends `PUT_CODE` and the code using `boot_put8`.

  5. The pi side copies the received code into memory starting at the
     indicated code address using `PUT8`(from step 2).  It then computes
     `crc32(addr,nbytes)` over this range and compares it to the
     expected value received in step 2.  If they do not match, it sends
     `BOOT_ERROR`.  If so, it sends back `BOOT_SUCCESS`.

  6. Once the Unix side receives `BOOT_SUCCESS` it simply echoes all
     subsequent received bytes to the terminal --- thus, any `printk`
     from the pi will appear on your Unix terminal.

  7. If at any point the pi side receives an unexpected message, it
     sends a `BOOT_ERROR` message to the unix side and reboots.

     If at any point the Unix side receives an unexpected or error message
     it simply exits with an error --- this means you'll have to manually
     reboot your pi before you can load anything.  We expect these errors
     to be rare, so do the manual reboot to keep the pi logic simple.
     You are certainly welcome to implement a time-out + reboot on the
     pi side!

The use of send-response in the protocol is intended to prevent the
Unix-side from overrunning the pi side's finite-sized UART queue (for
the device we use: 8 bytes).  The CRC and other checks guard against
corruption.

  - (Q6: Note: we should have been checksumming the header!    Why?)

I would start small: do each message, check that it works, then do
the next.  If you go in small, verifiable steps and check at each point,
this can go fairly smoothly.  If you do everything all-at-once and then
have to play wack-a-mole with various bugs that arise from combinations
of mistakes, this will take awhile.

And don't forget: there are a bunch of useful error checking macros in
`libunix/demand.h` and uses in `libunix/*.c`.
