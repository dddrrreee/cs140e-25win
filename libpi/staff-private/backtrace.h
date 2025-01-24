void backtrace_print(void);

// given framepointer reg, return name of first entry.
const char *backtrace_name_fp(uint32_t *fp);

const char *bt_recover_name(uint32_t *pc);

