#ifndef __MENU_FRAME_H__
#define __MENU_FRAME_H__


static inline void yed_menu_frame_add_name(yed_buffer *buff, char *name, int frame_width) {
    int i, len, n_spaces;

    len      = strlen(name);
    n_spaces = (frame_width / 2) - (len / 2);

    for (i = 0; i < n_spaces; i += 1) {
        yed_append_to_line_no_undo(buff, 1, ' ');
    }

    for (i = 0; i < len; i += 1) {
        if (name[i] == '\n')    { break; }

        yed_append_to_line_no_undo(buff, 1, name[i]);
    }
}

static inline void yed_menu_frame_add_text(yed_buffer *buff, char *text) {
    int i, row, len;

    yed_buffer_add_line_no_undo(buff);
    row = yed_buff_n_lines(buff);
    len = strlen(text);

    for (i = 0; i < len; i += 1) {
        if (text[i] == '\n') {
            yed_buffer_add_line_no_undo(buff);
            row = yed_buff_n_lines(buff);
        } else {
            yed_append_to_line(buff, row, text[i]);
        }
    }
}

static inline void yed_menu_frame_add_option(yed_buffer *buff, char *option) {
    int i, len;

    yed_buffer_add_line_no_undo(buff);
    len = strlen(option);

    for (i = 0; i < len; i += 1) {
        if (option[i] == '\n')    { break; }

        yed_append_to_line_no_undo(buff, yed_buff_n_lines(buff), option[i]);
    }
}

static inline void yed_menu_frame_make(char *name, char *text, int n_options, char **options) {
    yed_frame  *frame;
    yed_buffer *buff;
    int         i;

    /* Make the frame. */
    frame = yed_add_new_frame(0.15, 0.15, 0.7, 0.7);
    yed_clear_frame(frame);
    yed_activate_frame(frame);

    buff = yed_get_buffer("*menu-frame");

    if (!buff) {
        buff = yed_create_buffer("*menu-frame");
        buff->flags |= BUFF_RD_ONLY;
    } else {
        yed_buff_clear_no_undo(buff);
    }

    ASSERT(buff, "did not create '*menu-frame' buffer");

    yed_frame_set_buff(frame, buff);

    yed_menu_frame_add_name(buff, name, frame->width);

    yed_buffer_add_line_no_undo(buff);

    yed_menu_frame_add_text(buff, text);

    yed_buffer_add_line_no_undo(buff);

    if (n_options == 0) {
        yed_menu_frame_add_option(buff, "* Close");
    } else {
        for (i = 0; i < n_options; i += 1) {
            yed_menu_frame_add_option(buff, options[i]);
        }
    }
}

#define YED_MENU_FRAME_MAKE(name, text, ...)                    \
do {                                                            \
    char *__YMFM_options[] = { __VA_ARGS__ };                   \
    yed_menu_frame_make((name), (text),                         \
                        sizeof(__YMFM_options) / sizeof(char*), \
                        __YMFM_options);                        \
} while (0)


#endif
