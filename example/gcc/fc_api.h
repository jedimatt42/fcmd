#ifndef _FC_API_H
#define _FC_API_H 1

#define FC_SYS *(int *)0x6080
#define FC_TPUTC 0x6082
#define FC_TPUTS 0x6086
#define FC_GETSTR 0x608A

#define DECL_FC_API_CALL(index, func, return_type, arg_sig, args)     \
    static inline return_type func arg_sig                            \
    {                                                                 \
        __asm__("li r0,%0"                                            \
                :                                                     \
                : "i"(index));                                        \
        return_type(*tramp) arg_sig = (return_type(*) arg_sig)FC_SYS; \
        return tramp args;                                            \
    }

// declare function: void fc_tputc(int value);
DECL_FC_API_CALL(FC_TPUTC, fc_tputc, void, (int value), (value))

// declare function: void fc_tputs(char* value);
DECL_FC_API_CALL(FC_TPUTS, fc_tputs, void, (char *value), (value))

// declare function: void fc_getstr(int x, int y, char* var, int limit, int backspace)
DECL_FC_API_CALL(FC_GETSTR, fc_getstr, void, (int x, int y, char *var, int limit, int backspace), (x, y, var, limit, backspace))

#endif