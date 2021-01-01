#ifndef __CMD_LINE_H__
#define __CMD_LINE_H__

#define DEFAULT_CMD_PROMPT_STRING "YED> "

typedef struct yed_cmd_line_readline_t {
    array_t *hist;
    int      hist_item;
    char    *save_line;
    int      save_x;
} yed_cmd_line_readline;

void yed_cmd_line_readline_make(yed_cmd_line_readline *rdline, array_t *hist);
void yed_cmd_line_readline_reset(yed_cmd_line_readline *rdline, array_t *hist);

int yed_cmd_line_readline_take_key(yed_cmd_line_readline *rdline, int key);
char *yed_cmd_line_readline_get_string(void); /* free result!!! */

void yed_builtin_cmd_prompt_start(int n_args, char **args);
void yed_builtin_cmd_prompt_take_key(int key);

#endif
