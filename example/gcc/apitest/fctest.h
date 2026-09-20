#ifndef _FCTEST_H
#define _FCTEST_H 1

/*
 * Minimal self-check harness shared by the API test demos.
 *
 * Include with -I<path to this directory>.  Each test lives in a single .c
 * file, so the pass/fail/skip counters are static per program.
 *
 * Result protocol:
 *   [PASS] <description>
 *   [FAIL] <description> got=<n> want=<n>
 *   [SKIP] <reason>
 *   [OBSERVE] <what the human should look for>
 *   [WAIT] <prompt>
 *   pass=<n> fail=<n> skip=<n>
 *
 * main() should return fc_summary(), so a failed test surfaces as a
 * ForceCommand error (non-zero return).
 */

#include <fc_api.h>
#include <kscan.h>

static int fc_pass_count = 0;
static int fc_fail_count = 0;
static int fc_skip_count = 0;

/* str_from_uint shares one static buffer, so format and print one value at a
 * time rather than composing several calls in a single expression. */
static inline void fc_puts_int(int v) {
    if (v < 0) {
        term_putc('-');
        term_puts(str_from_uint((unsigned int)(0 - v)));
    } else {
        term_puts(str_from_uint((unsigned int)v));
    }
}

static inline void fc_pass_line(const char* label) {
    term_puts("[PASS] ");
    term_puts(label);
    term_putc('\n');
    fc_pass_count++;
}

static inline void fc_fail_line(const char* label) {
    term_puts("[FAIL] ");
    term_puts(label);
    term_putc('\n');
    fc_fail_count++;
}

static inline void fc_check(const char* label, int ok) {
    if (ok) {
        fc_pass_line(label);
    } else {
        fc_fail_line(label);
    }
}

static inline void fc_check_eq_int(const char* label, int got, int want) {
    if (got == want) {
        fc_pass_line(label);
    } else {
        term_puts("[FAIL] ");
        term_puts(label);
        term_puts(" got=");
        fc_puts_int(got);
        term_puts(" want=");
        fc_puts_int(want);
        term_putc('\n');
        fc_fail_count++;
    }
}

static inline void fc_skip(const char* reason) {
    term_puts("[SKIP] ");
    term_puts(reason);
    term_putc('\n');
    fc_skip_count++;
}

static inline void fc_observe(const char* what) {
    term_puts("[OBSERVE] ");
    term_puts(what);
    term_putc('\n');
}

/* Block until a fresh key is pressed. */
static inline void fc_wait(const char* prompt) {
    term_puts("[WAIT] ");
    term_puts(prompt);
    term_putc('\n');
    while ((KSCAN_STATUS & KSCAN_MASK) == 0) {
        term_kscan(5);
    }
    term_kscan(5);
}

static inline int fc_summary(void) {
    term_puts("pass=");
    term_puts(str_from_uint((unsigned int)fc_pass_count));
    term_puts(" fail=");
    term_puts(str_from_uint((unsigned int)fc_fail_count));
    term_puts(" skip=");
    term_puts(str_from_uint((unsigned int)fc_skip_count));
    term_putc('\n');
    return fc_fail_count ? 1 : 0;
}

#define FC_CHECK(cond) fc_check(#cond, (cond))
#define FC_CHECK_EQ(got, want) fc_check_eq_int(#got, (got), (want))
#define FC_SKIP(reason) fc_skip(reason)
#define FC_OBSERVE(what) fc_observe(what)
#define FC_WAIT(prompt) fc_wait(prompt)

#endif
