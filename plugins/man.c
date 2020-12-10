#include <yed/plugin.h>

void man(int n_args, char **args);
void man_word(int n_args, char **args);

yed_buffer *get_or_make_buff(void) {
    yed_buffer *buff;

    buff = yed_get_buffer("*man-page");

    if (buff == NULL) {
        buff = yed_create_buffer("*man-page");
        buff->flags |= BUFF_RD_ONLY | BUFF_SPECIAL;
    }

    return buff;
}

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_command(self, "man", man);
    yed_plugin_set_command(self, "man-word", man_word);

    return 0;
}

void man_word(int n_args, char **args) {
    char *word;

    word = yed_word_under_cursor();

    if (!word) {
        yed_cerr("cursor is not on a word");
        return;
    }

    YEXE("man", word);

    free(word);
}

void man(int n_args, char **args) {
    int        width;
    char       pre_cmd_buff[1024];
    char       cmd_buff[1024];
    char       err_buff[1024];
    int        i;
    int        status;
    FILE      *stream;

    pre_cmd_buff[0] = 0;
    cmd_buff[0]     = 0;
    err_buff[0]     = 0;

    strcat(pre_cmd_buff, "man --ascii");
    strcat(err_buff,     "man");
    for (i = 0; i < n_args; i += 1) {
        strcat(pre_cmd_buff, " ");
        strcat(pre_cmd_buff, args[i]);
        strcat(err_buff, " ");
        strcat(err_buff, args[i]);
    }
    strcat(pre_cmd_buff, " 2>/dev/null");

    strcat(cmd_buff, pre_cmd_buff);
    strcat(cmd_buff, " >/dev/null");

    if ((stream = popen(cmd_buff, "r")) == NULL) {
        yed_cerr("failed to invoke '%s'", cmd_buff);
        return;
    }
    status = pclose(stream);
    if (status) {
        yed_cerr("command '%s' failed", err_buff);
        return;
    }

    YEXE("special-buffer-prepare-focus", "*man-page");

    if (ys->active_frame != NULL) {
        width = ys->active_frame->width;
    } else {
        width = 80;
    }

    snprintf(cmd_buff, sizeof(cmd_buff),
             "bash -c 'MANWIDTH=%d ", width);
    strcat(cmd_buff, pre_cmd_buff);
    strcat(cmd_buff, "'");

    if (yed_read_subproc_into_buffer(cmd_buff, get_or_make_buff(), &status) != 0) {
        YEXE("special-buffer-prepare-unfocus", "*man-page");
        yed_cerr("failed to invoke '%s'", cmd_buff);
        return;
    }
    if (status != 0) {
        YEXE("special-buffer-prepare-unfocus", "*man-page");
        yed_cerr("command '%s' failed", err_buff);
        return;
    }

    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
    YEXE("buffer", "*man-page");
}
