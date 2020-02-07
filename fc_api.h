#define API_TABLE 0x6070

extern volatile int* tramp_data;

extern void* trampoline();

#define DECLARE_BANKED(realname, bank, return_type, banked_name, param_types, param_list) \
__attribute__ ((gnu_inline, always_inline)) \
static inline return_type banked_name param_types { \
  static const int foo[]={(int)MYBANK, (int)realname, (int)bank}; \
  tramp_data = (int*) foo; \
  return (return_type) trampoline param_list; \
}

#define DECLARE_BANKED_VOID(realname, bank, banked_name, param_types, param_list) \
__attribute__ ((gnu_inline, always_inline)) \
static inline void banked_name param_types { \
  static const int foo[]={(int)MYBANK, (int)realname, (int)bank}; \
  tramp_data = (int*) foo; \
  trampoline param_list; \
}
