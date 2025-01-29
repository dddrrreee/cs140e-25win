### What to do

You'll have to implement:
  - `libunix/read-file.c`: figure out how big a file is, allocate memory for it, read it in.
  - `libunix/find-ttyusb.c`: look up the usb devices on your laptop and pick out the one 
     used by your pi and return its name.

Checking:
  - "make check" can check the two read file tests (since there are two .out files)
  - "make run" will run the programs.
  - can't currently autocheck the tty, so you'll have to figure this yourself.
