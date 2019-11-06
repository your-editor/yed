#include "plugin.h"

void latex_compile_current_file(int n_args, char **args);
void latex_view_current_file(int n_args, char **args);

void add_commands(yed_plugin *self);

int yed_plugin_boot(yed_plugin *self) {
    add_commands(self);
    return 0;
}

void add_commands(yed_plugin *self) {
    yed_plugin_set_command(self, "latex-compile-current-file", latex_compile_current_file);
    yed_plugin_set_command(self, "latex-view-current-file",    latex_view_current_file);
}

void latex_compile_current_file(int n_args, char **args) {
    char       *comp_prg,
               *path;
    char        cmd_buff[256];
    yed_frame  *frame;
    yed_buffer *buff;

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (buff->kind != BUFF_KIND_FILE || !buff->path) {
        yed_append_text_to_cmd_buff("[!] buffer is not a file");
        return;
    }

    path = buff->path;

    comp_prg = yed_get_var("latex-comp-prg");

    if (!comp_prg) {
        yed_append_text_to_cmd_buff("[!] 'latex-comp-prg' not set");
        return;
    }

    cmd_buff[0] = 0;

    strcat(cmd_buff, comp_prg);
    strcat(cmd_buff, " ");
    strcat(cmd_buff, path);
    strcat(cmd_buff, " | less");

    YEXE("sh", cmd_buff);
}

void latex_view_current_file(int n_args, char **args) {
    char       *view_prg,
               *path;
    char        cmd_buff[256];
    yed_frame  *frame;
    yed_buffer *buff;
    int         err;

    if (!ys->active_frame) {
        yed_append_text_to_cmd_buff("[!] no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_append_text_to_cmd_buff("[!] active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (buff->kind != BUFF_KIND_FILE || !buff->path) {
        yed_append_text_to_cmd_buff("[!] buffer is not a file");
        return;
    }

    path = buff->path;

    view_prg = yed_get_var("latex-view-prg");

    if (!view_prg) {
        yed_append_text_to_cmd_buff("[!] 'latex-view-prg' not set");
        return;
    }

    cmd_buff[0] = 0;

    path = path_without_ext(path);

    strcat(cmd_buff, view_prg);
    strcat(cmd_buff, " ");
    strcat(cmd_buff, path);
    strcat(cmd_buff, ".pdf");
    free(path);

    err = system(cmd_buff);
    if (err != 0) {
        yed_append_text_to_cmd_buff("'");
        yed_append_text_to_cmd_buff(cmd_buff);
        yed_append_text_to_cmd_buff("' exited with non-zero status ");
        yed_append_int_to_cmd_buff(err);
    }
}
