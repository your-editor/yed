#ifndef __TERM_H__
#define __TERM_H__

#define TERM_BLACK                   "\e[0;30m"
#define TERM_BLUE                    "\e[0;34m"
#define TERM_GREEN                   "\e[0;32m"
#define TERM_CYAN                    "\e[0;36m"
#define TERM_RED                     "\e[0;31m"
#define TERM_PURPLE                  "\e[0;35m"
#define TERM_BROWN                   "\e[0;33m"
#define TERM_GRAY                    "\e[0;37m"
#define TERM_DARK_GRAY               "\e[1;30m"
#define TERM_LIGHT_BLUE              "\e[1;34m"
#define TERM_LIGHT_GREEN             "\e[1;32m"
#define TERM_LIGHT_CYAN              "\e[1;36m"
#define TERM_LIGHT_RED               "\e[1;31m"
#define TERM_LIGHT_PURPLE            "\e[1;35m"
#define TERM_YELLOW                  "\e[1;33m"
#define TERM_WHITE                   "\e[1;37m"
#define TERM_BG_BLACK                "\e[0;40m"
#define TERM_BG_BLUE                 "\e[0;44m"
#define TERM_BG_GREEN                "\e[0;42m"
#define TERM_BG_CYAN                 "\e[0;46m"
#define TERM_BG_RED                  "\e[0;41m"
#define TERM_BG_PURPLE               "\e[0;45m"
#define TERM_BG_GREY                 "\e[0;47m"
#define TERM_BG_WHITE                "\e[1;47m"
#define TERM_INVERSE                 "\e[7m"
#define TERM_SAVE                    "\e7"
#define TERM_RESTORE                 "\e8"
#define TERM_RESET                   "\e[0m"

#define TERM_ALT_SCREEN              "\e[?1049h"
#define TERM_STD_SCREEN              "\e[?1049l"
#define TERM_CLEAR_SCREEN            "\e[2J"
#define TERM_CLEAR_LINE_L            "\e[1K"
#define TERM_CLEAR_LINE_R            "\e[0K"
#define TERM_CLEAR_LINE              "\e[2K"
#define TERM_SCROLL_UP               "\e[1U"
#define TERM_SCROLL_DOWN             "\e[1S"

#define TERM_CURSOR_HOME             "\e[H"
#define TERM_CURSOR_HIDE             "\e[?25l"
#define TERM_CURSOR_SHOW             "\e[?25h"
#define TERM_CURSOR_MOVE_BEG         "\e["
#define TERM_CURSOR_MOVE_SEP         ";"
#define TERM_CURSOR_MOVE_END         "H"
#define TERM_ENABLE_BRACKETED_PASTE  "\e[?2004h"
#define TERM_DISABLE_BRACKETED_PASTE "\e[?2004l"

#define TERM_MOUSE_BUTTON_ENABLE     "\e[?1002h"
#define TERM_MOUSE_BUTTON_DISABLE    "\e[?1002l"
#define TERM_MOUSE_ANY_ENABLE        "\e[?1003h"
#define TERM_MOUSE_ANY_DISABLE       "\e[?1003l"
#define TERM_SGR_1006_ENABLE         "\e[?1006h"
#define TERM_SGR_1006_DISABLE        "\e[?1006l"

#define TERM_DEFAULT_READ_TIMEOUT (3)

#define TERM_CURSOR_STYLE_DEFAULT            (0)
#define TERM_CURSOR_STYLE_BLINKING_BLOCK     (1)
#define TERM_CURSOR_STYLE_STEADY_BLOCK       (2)
#define TERM_CURSOR_STYLE_BLINKING_UNDERLINE (3)
#define TERM_CURSOR_STYLE_STEADY_UNDERLINE   (4)
#define TERM_CURSOR_STYLE_BLINKING_BAR       (5)
#define TERM_CURSOR_STYLE_STEADY_BAR         (6)

int yed_term_enter(void);
int yed_term_exit(void);

void yed_term_timeout_on(void);
void yed_term_timeout_off(void);
void yed_term_set_timeout(int n_x_100_ms);

int yed_term_get_dim(int *r, int *c);
int yed_term_says_it_supports_truecolor(void);

void yed_term_set_cursor_style(int style);

void yed_register_sigwinch_handler(void);
void yed_register_sigstop_handler(void);
void yed_register_sigcont_handler(void);
void yed_register_sigterm_handler(void);
void yed_register_sigquit_handler(void);
void yed_register_sigsegv_handler(void);
void yed_register_sigabrt_handler(void);
void yed_register_sigill_handler(void);
void yed_register_sigfpe_handler(void);
void yed_register_sigbus_handler(void);
void yed_register_sigchld_handler(void);
void yed_register_sigpipe_handler(void);
int yed_check_for_resize(void);
void yed_handle_resize(void);
void yed_term_enable_mouse_reporting(void);
void yed_term_disable_mouse_reporting(void);
int yed_term_mouse_reporting_enabled(void);

#endif
