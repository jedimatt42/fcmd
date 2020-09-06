#define FC_SYS *(int *)0x6080
#define FC_TPUTC 0x6082
#define FC_TPUTS 0x6086

#define DECL_FC_API_CALL(index, func, return_type, arg_sig, args)     \
    static inline return_type func arg_sig                            \
    {                                                                 \
        __asm__("li r0,%0"                                            \
                :                                                     \
                : "i"(index));                                        \
        return_type(*tramp) arg_sig = (return_type(*) arg_sig)FC_SYS; \
        return tramp args;                                            \
    }

// declare function: void fc_tputs(char* value);
DECL_FC_API_CALL(FC_TPUTS, fc_tputs, void, (char *value), (value))
