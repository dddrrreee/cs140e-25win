#include "rpi.h"
#include "mbr.h"

void notmain() {
  kmalloc_init(8);
  pi_sd_init();

  trace("Reading MBR...\n");
  mbr_t *mbr = mbr_read();
  trace("Success!\n");

  trace("Checking MBR...\n");
  mbr_check(mbr);
  trace("Success!\n");

  trace("Looking for Partition 0...\n");
  mbr_partition_ent_t part = mbr_get_partition(mbr, 0);
  assert(!mbr_partition_empty(&part));
  assert(mbr_part_is_fat32(part.part_type));
  mbr_partition_print("Partition 0", &part);

  printk("PASS: %s\n\n", __FILE__);
}
