#include "rpi.h"
#include "pi-sd.h"

#define SECTOR_SIZE 512

void notmain() {
  kmalloc_init(8);

  trace("Starting SD Driver test.\n");

  trace("Initializing...\n");
  pi_sd_init();

  trace("Reading first block...\n");
  char *buffer = kmalloc(SECTOR_SIZE);
  assert(pi_sd_read(buffer, 0, 1));

  trace("Verifying signature at the end of the block...\n");
  // Endianness: we want 0xaa55, which is actually [0x55, 0xaa]
  demand(buffer[510] == 0x55, "Expected buf[510]=0x55, got %x", buffer[510]);
  demand(buffer[511] == 0xaa, "Expected buf[510]=0xaa, got %x", buffer[511]);
  trace("Got 0x%x!\n", *(uint16_t *)(&buffer[510]));
  printk("PASS: %s\n", __FILE__);
}
