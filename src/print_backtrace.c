#ifdef HAS_BACKTRACE

#include <execinfo.h>

void print_backtrace(void) {
    int    n;
    void  *bt_buff[MAX_BT_LEN];
    char **strs;
    int    i;

    n    = backtrace(bt_buff, MAX_BT_LEN);
    strs = backtrace_symbols(bt_buff, n);

    if (strs == NULL) {
        printf("backtrace unavailable\n");
        return;
    }

    for (i = 3; i < n; i += 1) {
        printf("    %s\n", strs[i]);
    }

    free(strs);
}

#endif
