#include "fctest.h"

/*
 * I1 - system information, SAMS information, clock, and hex conversions.
 *
 * Covers: sys_info, sys_sams_info, time_get, hex_from_uint, hex_to_int.
 */

struct HexCase {
    unsigned int value;
    const char* text;
};

static const struct HexCase hex_cases[] = {
    { 0x0000, "0000" },
    { 0x0001, "0001" },
    { 0x000F, "000F" },
    { 0x0010, "0010" },
    { 0x00FF, "00FF" },
    { 0xABCD, "ABCD" },
    { 0xFFFF, "FFFF" }
};

static void test_hex_from_uint(void) {
    unsigned int i;
    for (i = 0; i < sizeof(hex_cases) / sizeof(hex_cases[0]); i++) {
        char* got = hex_from_uint(hex_cases[i].value);
        term_puts("hex 0x");
        term_puts(hex_cases[i].text);
        term_puts(" -> ");
        term_puts(got);
        term_putc('\n');
        FC_CHECK_EQ(str_cmp(got, hex_cases[i].text), 0);
        FC_CHECK_EQ(hex_to_int(got), (int)hex_cases[i].value);
    }
}

static void test_hex_to_int(void) {
    FC_CHECK_EQ(hex_to_int("0000"), 0);
    FC_CHECK_EQ(hex_to_int("7FFF"), 0x7FFF);
    FC_CHECK_EQ(hex_to_int("+10"), 16);
    FC_CHECK_EQ(hex_to_int(" FF"), 255);
    FC_CHECK_EQ(hex_to_int("-1"), -1);
    FC_CHECK_EQ(hex_to_int("GG"), 0);
    FC_CHECK_EQ(hex_to_int(""), 0);
}

static void test_sys_info(void) {
    struct SystemInformation info;
    sys_info(&info);

    FC_CHECK(info.dsrList != 0);
    FC_CHECK(info.currentDsr != 0);
    FC_CHECK(info.currentPath != 0);

    if (info.dsrList != 0 && info.currentDsr != 0) {
        FC_CHECK(info.currentDsr >= info.dsrList);
        FC_CHECK(info.currentDsr < info.dsrList + 20);
    } else {
        FC_SKIP("dsr list not populated");
    }

    term_puts("current path: ");
    term_puts(info.currentPath);
    term_putc('\n');
    term_puts("vdp io buffer: 0x");
    term_puts(hex_from_uint(info.vdp_io_buf));
    term_putc('\n');
}

static void test_sams(void) {
    struct SamsInformation sams;
    sys_sams_info(&sams);

    term_puts("sams next=");
    term_puts(str_from_uint((unsigned int)sams.next_page));
    term_puts(" total=");
    term_puts(str_from_uint((unsigned int)sams.total_pages));
    term_putc('\n');

    FC_CHECK(sams.total_pages >= 0);
    FC_CHECK(sams.next_page >= 0);

    if (sams.total_pages == 0) {
        FC_SKIP("no SAMS detected");
    } else {
        FC_CHECK(sams.next_page <= sams.total_pages);
    }
}

static void test_time(void) {
    struct DateTime dt;
    time_get(&dt);

    if (dt.year == 0 && dt.month == 0) {
        FC_SKIP("no clock detected");
        return;
    }

    FC_CHECK(dt.month >= 1 && dt.month <= 12);
    FC_CHECK(dt.day >= 1 && dt.day <= 31);
    FC_CHECK(dt.hours >= 1 && dt.hours <= 12);
    FC_CHECK(dt.minutes >= 0 && dt.minutes <= 59);
    FC_CHECK(dt.seconds >= 0 && dt.seconds <= 59);
    FC_CHECK(dt.pm == 0 || dt.pm == 1);

    term_puts("[OBSERVE] clock reads ");
    fc_puts_int((int)dt.year);
    term_putc('-');
    fc_puts_int((int)dt.month);
    term_putc('-');
    fc_puts_int((int)dt.day);
    term_putc(' ');
    fc_puts_int((int)dt.hours);
    term_putc(':');
    if (dt.minutes < 10) {
        term_putc('0');
    }
    fc_puts_int((int)dt.minutes);
    term_putc(':');
    if (dt.seconds < 10) {
        term_putc('0');
    }
    fc_puts_int((int)dt.seconds);
    term_puts(dt.pm ? " pm\n" : " am\n");
}

int main(char* args) {
    (void)args;

    term_puts("SYSINFO test\n");

    test_hex_from_uint();
    test_hex_to_int();
    test_sys_info();
    test_sams();
    test_time();

    return fc_summary();
}
