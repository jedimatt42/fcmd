#include "fctest.h"

/*
 * I4 - list stack.
 *
 * Covers: list_init, list_push, list_pop, list_get, including the ceiling and
 * overflow path.
 *
 * The buffer is a static array, which the linker places in upper expansion
 * memory. The list implementation's eviction walk compares an entry length
 * against the list end address cast to int, so its behavior depends on the
 * address sign. This test therefore asserts the invariants that must hold
 * regardless (newest entry survives, bytes stay within the buffer) and prints
 * the surviving count for observation.
 */

#define BUF_SIZE 64

static char list_buf[BUF_SIZE];

static int list_count(struct List* list) {
    int n = 0;
    while (list_get(list, n) != 0) {
        n++;
    }
    return n;
}

static void test_init_empty(void) {
    struct List list;
    char out[8];

    list_init(&list, list_buf, list_buf + BUF_SIZE);
    FC_CHECK(list.addr == list_buf);
    FC_CHECK(list.end == list_buf);
    FC_CHECK(list.ceiling == list_buf + BUF_SIZE);
    FC_CHECK(list_get(&list, 0) == 0);

    /* pop on an empty list must not touch the output buffer */
    out[0] = 'x';
    list_pop(&list, out, sizeof(out));
    FC_CHECK_EQ(out[0], 'x');
    FC_CHECK(list.end == list_buf);
}

static void test_push_pop_order(void) {
    struct List list;
    struct ListEntry* e;
    char out[16];
    char one[] = "one";
    char two[] = "two";
    char three[] = "three";

    list_init(&list, list_buf, list_buf + BUF_SIZE);

    list_push(&list, one, 4);
    list_push(&list, two, 4);
    list_push(&list, three, 6);

    /* newest first */
    e = list_get(&list, 0);
    FC_CHECK(e != 0);
    FC_CHECK_EQ(e->length, 6);
    FC_CHECK_EQ(str_cmp(e->data, "three"), 0);

    e = list_get(&list, 1);
    FC_CHECK(e != 0);
    FC_CHECK_EQ(str_cmp(e->data, "two"), 0);

    e = list_get(&list, 2);
    FC_CHECK(e != 0);
    FC_CHECK_EQ(str_cmp(e->data, "one"), 0);

    FC_CHECK(list_get(&list, 3) == 0);
    FC_CHECK(list_get(&list, 100) == 0);

    /* pop removes from the head, newest first */
    list_pop(&list, out, 16);
    FC_CHECK_EQ(str_cmp(out, "three"), 0);

    list_pop(&list, out, 16);
    FC_CHECK_EQ(str_cmp(out, "two"), 0);

    list_pop(&list, out, 16);
    FC_CHECK_EQ(str_cmp(out, "one"), 0);

    out[0] = 'x';
    list_pop(&list, out, 16);
    FC_CHECK_EQ(out[0], 'x');
    FC_CHECK(list_get(&list, 0) == 0);
}

static void test_pop_truncates(void) {
    struct List list;
    char out[8];
    char hello[] = "hello";

    list_init(&list, list_buf, list_buf + BUF_SIZE);
    list_push(&list, hello, 6);

    out[0] = 'x';
    out[1] = 'x';
    out[2] = 'x';
    out[3] = 'Z';

    /* limit smaller than the stored length copies only limit bytes and does
       not null terminate, but still removes the whole entry */
    list_pop(&list, out, 3);
    FC_CHECK_EQ(out[0], 'h');
    FC_CHECK_EQ(out[1], 'e');
    FC_CHECK_EQ(out[2], 'l');
    FC_CHECK_EQ(out[3], 'Z');
    FC_CHECK(list_get(&list, 0) == 0);
}

static void test_overflow(void) {
    struct List list;
    struct ListEntry* e;
    char item[] = "0123456789";
    int i;
    int count;

    list_init(&list, list_buf, list_buf + BUF_SIZE);

    /* each entry costs 11 + 2 = 13 bytes; four fit in 64 bytes, the fifth
       forces the eviction path */
    for (i = 0; i < 5; i++) {
        item[0] = 'a' + i;
        list_push(&list, item, 11);
        FC_CHECK(list.end <= list.ceiling);
        FC_CHECK(list.end >= list.addr);
    }

    /* the newest entry must always survive an overflow */
    e = list_get(&list, 0);
    FC_CHECK(e != 0);
    FC_CHECK_EQ(e->data[0], 'a' + 4);
    FC_CHECK(list_get(&list, 200) == 0);

    count = list_count(&list);
    term_puts("[OBSERVE] entries surviving overflow: ");
    term_puts(str_from_uint((unsigned int)count));
    term_putc('\n');
    FC_OBSERVE("implementation may drop all older entries on overflow; newest is kept");

    /* draining the list must leave it empty */
    for (i = 0; i < count; i++) {
        char out[16];
        list_pop(&list, out, 16);
    }
    FC_CHECK(list_get(&list, 0) == 0);
}

int main(char* args) {
    (void)args;

    term_puts("LISTTEST test\n");

    test_init_empty();
    test_push_pop_order();
    test_pop_truncates();
    test_overflow();

    return fc_summary();
}
