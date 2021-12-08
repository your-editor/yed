#include <yed/plugin.h>

void latex_compile_current_file(int n_args, char **args);
void latex_view_current_file(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_command(self, "latex-compile-current-file", latex_compile_current_file);
    yed_plugin_set_command(self, "latex-view-current-file",    latex_view_current_file);

    return 0;
}

void latex_compile_current_file(int n_args, char **args) {
    char       *comp_prg,
               *update_view_prg,
               *path;
    char        cmd_buff[256];
    int         len;
    yed_frame  *frame;
    yed_buffer *buff;

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (buff->kind != BUFF_KIND_FILE || !buff->path) {
        yed_cerr("buffer is not a file");
        return;
    }

    path = buff->path;

    comp_prg = yed_get_var("latex-comp-prg");

    if (!comp_prg) {
        yed_cerr("'latex-comp-prg' not set");
        return;
    }

    cmd_buff[0] = 0;

    len = perc_subst(comp_prg, path, cmd_buff, sizeof(cmd_buff));
    ASSERT(len >= 0, "buff too small for perc_subst");

    YEXE("less", cmd_buff);

    update_view_prg = yed_get_var("latex-update-view-prg");
    if (update_view_prg) {
        cmd_buff[0] = 0;
        len = perc_subst(update_view_prg, path, cmd_buff, sizeof(cmd_buff));
        ASSERT(len >= 0, "buff too small for perc_subst");
        YEXE("sh-silent", cmd_buff);
    }
}

void latex_view_current_file(int n_args, char **args) {
    char       *view_prg,
               *path,
               pdf_path_buff[256];
    int         len;
    char        cmd_buff[256];
    yed_frame  *frame;
    yed_buffer *buff;

    if (!ys->active_frame) {
        yed_cerr("no active frame");
        return;
    }

    frame = ys->active_frame;

    if (!frame->buffer) {
        yed_cerr("active frame has no buffer");
        return;
    }

    buff = frame->buffer;

    if (buff->kind != BUFF_KIND_FILE || !buff->path) {
        yed_cerr("buffer is not a file");
        return;
    }

    path = buff->path;
    path = path_without_ext(path);
    pdf_path_buff[0] = 0;
    strcat(pdf_path_buff, path);
    strcat(pdf_path_buff, ".pdf");

    view_prg = yed_get_var("latex-view-prg");

    if (!view_prg) {
        yed_cerr("'latex-view-prg' not set");
        return;
    }

    cmd_buff[0] = 0;

    len = perc_subst(view_prg, pdf_path_buff, cmd_buff, sizeof(cmd_buff));
    ASSERT(len >= 0, "buff too small for perc_subst");

    free(path);

    YEXE("sh-silent", cmd_buff);
}
