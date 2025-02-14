// in 0-user-sp-lr-asm.S

//*******************************************************
// routines to get and set user <sp> and <lr> registers
// using the cps instruction and switching to SYSTEM.

// get USER <sp> by using the <cps> instruction to switch 
// to SYSTEM mode and back.  
uint32_t cps_user_sp_get(void);
void cps_user_sp_set(uint32_t sp);

// get USER <lr> by using <cps> to switch to SYSTEM
// and back.
uint32_t cps_user_lr_get(void);
void cps_user_lr_set(uint32_t lr);

// get both at once with a single call.
uint32_t cps_user_sp_lr_get(uint32_t sp_lr[2]);
void cps_user_sp_lr_set(uint32_t sp_lr[2]);

//*******************************************************
// routines to get and set user <sp> and <lr> registers
// using the ldm/stm with the carat "^" operator.  should
// be faster than switching modes.

// use ldm/stm with the ^ modifier to get USER mode <sp>
void mem_user_sp_get(uint32_t *sp);
void mem_user_sp_set(const uint32_t *sp);

// use ldm/stm with the ^ modifier to get USER mode <lr>
void mem_user_lr_get(uint32_t *sp);
void mem_user_lr_set(const uint32_t *sp);

// use ldm/stm with the ^ modifier to get USER 
// mode <lr> and <sp>
void mem_user_sp_lr_get(uint32_t sp_lr[2]);
void mem_user_sp_lr_set(const uint32_t sp_lr[2]);

//************************************************************
// final routine: used to get <sp> and <lr> from arbitrary
// modes.   
//   0. record the original mode in a caller reg.
//   1. switch to the <mode> using:
//      msr cpsr_c, <mode reg>
//   2. get the SP register and write into the memory pointed
//      to by <sp> 
//   3. get the LR register and write into the memory pointed
//      to by <lr> 
//   4. switch back to the original mode.
//
// as an extension you can make a version that returns a 
// two element structure --- measure if faster!
void mode_get_lr_sp_asm(uint32_t mode, uint32_t *sp, uint32_t *lr);
