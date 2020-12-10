#include <yed/plugin.h>

static yed_buffer *buff;

void man(int n_args, char **args);
void man_word(int n_args, char **args);

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
    char       cmd_buff[1024];
    char       err_buff[1024];
    int        i;
    FILE      *stream;
    int        status;

    YEXE("special-buffer-prepare-focus", "*man-page");

    if (ys->active_frame != NULL) {
        width = ys->active_frame->width;
    } else {
        width = 80;
    }

    cmd_buff[0] = 0;
    err_buff[0] = 0;
    snprintf(cmd_buff, sizeof(cmd_buff),
             "bash -c 'MANWIDTH=%d man --ascii", width);
    strcat(err_buff, "man");
    for (i = 0; i < n_args; i += 1) {
        strcat(cmd_buff, " ");
        strcat(cmd_buff, args[i]);
        strcat(err_buff, " ");
        strcat(err_buff, args[i]);
    }
    strcat(cmd_buff, " 2>/dev/null'");

    if ((stream = popen(cmd_buff, "r")) == NULL) {
        yed_cerr("failed to invoke '%s'", cmd_buff);
        YEXE("special-buffer-prepare-unfocus", "*man-page");
        return;
    }

    if (buff == NULL) {
        buff = yed_create_buffer("*man-page");
        buff->flags |= BUFF_RD_ONLY | BUFF_SPECIAL;
    } else {
        yed_buff_clear_no_undo(buff);
    }

    status = yed_fill_buff_from_file_stream(buff, stream);
    if (status != BUFF_FILL_STATUS_SUCCESS) {
        yed_cerr("failed to create buffer *man-pages");
        YEXE("special-buffer-prepare-unfocus", "*man-page");
        return;
    }
    status = pclose(stream);

    if (status) {
        yed_cerr("command '%s' failed", err_buff);
        YEXE("special-buffer-prepare-unfocus", "*man-page");
        return;
    }

    yed_set_cursor_far_within_frame(ys->active_frame, 1, 1);
    YEXE("buffer", "*man-page");
}
