#include <yed/plugin.h>
#include <yed/highlight.h>

#define ARRAY_LOOP(a) for (__typeof((a)[0]) *it = (a); it < (a) + (sizeof(a) / sizeof((a)[0])); ++it)

highlight_info hinfo;

void unload(yed_plugin *self);
void syntax_sh_line_handler(yed_event *event);
void syntax_sh_frame_handler(yed_event *event);
void syntax_sh_buff_mod_pre_handler(yed_event *event);
void syntax_sh_buff_mod_post_handler(yed_event *event);


int yed_plugin_boot(yed_plugin *self) {
    yed_event_handler frame, line, buff_mod_pre, buff_mod_post;
    char              *kwds[] = {
        "if",       "fi",       "do",       "in",
        "for",
        "done",     "else",     "elif",     "then",     "wait",   "case",   "esac",   "time"
        "while",    "until",    "shift",    "break",
        "source",   "select",   "export",
        "continue", "function",
    };
    char              *builtins[] = {
        "bg",        "cd",       "fc",       "fg",
        "let",       "pwd",      "set",
        "bind",      "dirs",     "echo",     "eval",    "exec",    "exit",    "false",   "hash",    "help",    "jobs",    "kill",    "popd", "read", "test", "trap", "true", "type", "wait"
        "alias",     "break",    "local",    "pushd",   "shopt",   "times",   "umask",   "unset",
        "caller",    "disown",   "enable",   "logout",  "printf",  "return",  "ulimit",
        "builtin",   "command",  "compgen",  "compopt", "declare", "getopts", "history", "mapfile", "suspend", "typeset", "unalias",
        "complete",  "readonly",
        "readarray",
    };

    yed_plugin_set_unload_fn(self, unload);


    frame.kind          = EVENT_FRAME_PRE_BUFF_DRAW;
    frame.fn            = syntax_sh_frame_handler;
    line.kind           = EVENT_LINE_PRE_DRAW;
    line.fn             = syntax_sh_line_handler;
    buff_mod_pre.kind   = EVENT_BUFFER_PRE_MOD;
    buff_mod_pre.fn     = syntax_sh_buff_mod_pre_handler;
    buff_mod_post.kind  = EVENT_BUFFER_POST_MOD;
    buff_mod_post.fn    = syntax_sh_buff_mod_post_handler;

    yed_plugin_add_event_handler(self, frame);
    yed_plugin_add_event_handler(self, line);
    yed_plugin_add_event_handler(self, buff_mod_pre);
    yed_plugin_add_event_handler(self, buff_mod_post);


    highlight_info_make(&hinfo);

    ARRAY_LOOP(kwds)
        highlight_add_kwd(&hinfo, *it, HL_KEY);
    ARRAY_LOOP(builtins)
        highlight_add_kwd(&hinfo, *it, HL_CALL);
    highlight_to_eol_from(&hinfo, "#", HL_COMMENT);
    highlight_within_multiline(&hinfo, "\"", "\"", '\\', HL_STR);
    highlight_prefixed_words_inclusive(&hinfo, '$', HL_CON);
    highlight_within(&hinfo, "$(", ")", 0, -1, HL_CON);
    highlight_within(&hinfo, "${", "}", 0, -1, HL_CON);
    highlight_numbers(&hinfo);

    ys->redraw = 1;

    return 0;
}

void unload(yed_plugin *self) {
    highlight_info_free(&hinfo);
    ys->redraw = 1;
}

void syntax_sh_frame_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_SH) {
        return;
    }

    highlight_frame_pre_draw_update(&hinfo, event);
}

void syntax_sh_line_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_SH) {
        return;
    }

    highlight_line(&hinfo, event);
}

void syntax_sh_buff_mod_pre_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_SH) {
        return;
    }

    highlight_buffer_pre_mod_update(&hinfo, event);
}

void syntax_sh_buff_mod_post_handler(yed_event *event) {
    yed_frame *frame;

    frame = event->frame;

    if (!frame
    ||  !frame->buffer
    ||  frame->buffer->kind != BUFF_KIND_FILE
    ||  frame->buffer->file.ft != FT_SH) {
        return;
    }

    highlight_buffer_post_mod_update(&hinfo, event);
}
