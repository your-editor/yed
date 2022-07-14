void yed_init_log(void);

#define _LOG_RESET() ys->cur_log_name = NULL

#define _LOG_ENTER(nm)                        \
do {                                          \
    char *__log_nm = nm;                      \
    array_push(ys->log_name_stack, __log_nm); \
} while (0)

#define LOG_EXIT()                 \
do {                               \
    array_pop(ys->log_name_stack); \
    _LOG_RESET();                  \
} while (0)

#define LOG_FN_ENTER()     _LOG_ENTER((char*)__FUNCTION__)
#define LOG_CMD_ENTER(cmd) _LOG_ENTER(cmd)

int yed_vlog(const char *fmt, va_list args);
int yed_log(const char *fmt, ...);
const char *yed_top_log_name(void);
yed_buffer *yed_get_log_buffer(void);
