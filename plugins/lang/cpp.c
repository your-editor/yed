#include <yed/plugin.h>

void unload(yed_plugin *self);
void maybe_change_ft(yed_buffer *buff);
void maybe_change_ft_event(yed_event *event);

int yed_plugin_boot(yed_plugin *self) {
    tree_it(yed_buffer_name_t, yed_buffer_ptr_t) bit;
    yed_event_handler                            buff_post_load_handler;
    yed_event_handler                            buff_pre_write_handler;

    YED_PLUG_VERSION_CHECK();

LOG_FN_ENTER();
    yed_plugin_set_unload_fn(self, unload);

    if (yed_plugin_make_ft(self, "C++") == FT_ERR_TAKEN) {
        yed_cerr("lang/cpp: unable to create file type name");
        LOG_EXIT();
        return 1;
    }

    buff_post_load_handler.kind = EVENT_BUFFER_POST_LOAD;
    buff_post_load_handler.fn   = maybe_change_ft_event;
    buff_pre_write_handler.kind = EVENT_BUFFER_PRE_WRITE;
    buff_pre_write_handler.fn   = maybe_change_ft_event;

    yed_plugin_add_event_handler(self, buff_post_load_handler);
    yed_plugin_add_event_handler(self, buff_pre_write_handler);

    tree_traverse(ys->buffers, bit) {
        maybe_change_ft(tree_it_val(bit));
    }

    YEXE("plugin-load", "lang/syntax/cpp");

LOG_EXIT();
    return 0;
}

void unload(yed_plugin *self) {
    YEXE("plugin-unload", "lang/syntax/cpp");
}

int kwd_scan(yed_buffer *buff) {
    yed_line *line;
    char     *line_data;
    int       line_len;
    char     *scan;
    int       pos;
    int       i;

    bucket_array_traverse(buff->lines, line) {
        array_zero_term(line->chars);
        line_data = array_data(line->chars);
        line_len  = array_len(line->chars);

        /************    std::    ************/
        scan = strstr(line_data, "std::");
        if (scan != NULL) { return 1; }

        /************    public:    ************/
        scan = strstr(line_data, "public:");
        if (scan != NULL) { return 1; }
        /************    private:    ************/
        scan = strstr(line_data, "private:");
        if (scan != NULL) { return 1; }
        /************    protected:    ************/
        scan = strstr(line_data, "protected:");
        if (scan != NULL) { return 1; }

        /************    template\s*<    ************/
        scan = strstr(line_data, "template");
        if (scan != NULL) {
            scan += strlen("template");
            pos = scan - line_data;
            while (pos < line_len && isspace(*scan)) { scan += 1; pos += 1; }
            if (pos < line_len && *scan == '<') { return 1; }
        }

        /************    class\s*\w    ************/
        scan = strstr(line_data, "class");
        if (scan != NULL) {
            scan += strlen("class");
            pos = scan - line_data;
            while (pos < line_len && isspace(*scan)) { scan += 1; pos += 1; }
            if (pos < line_len && (*scan == '_' || isalpha(*scan))) { return 1; }
        }

        /************    #include\s*<xxx>    ************/
        scan = strstr(line_data, "#include");
        if (scan != NULL) {
            scan += strlen("#include");
            pos = scan - line_data;
            while (pos < line_len && isspace(*scan)) { scan += 1; pos += 1; }
            if (pos < line_len && *scan == '<') {
                scan += 1;
                pos = scan - line_data;
                if (pos < line_len) {
                    char *cpp_includes[] = {
                        "cstdint",
                        "string",
                        "vector",
                        "map",
                        "list",
                        "array",
                        "bitset",
                        "queue",
                        "stack",
                        "forward_list",
                        "unordered_map",
                        "unordered_set",
                    };

                    for (i = 0; i < sizeof(cpp_includes) / sizeof(char*); i += 1) {
                        if (strncmp(scan, cpp_includes[i], strlen(cpp_includes[i])) == 0) {
                            return 1;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

void maybe_change_ft(yed_buffer *buff) {
    char *ext;

    if (buff->ft != FT_UNKNOWN
    &&  buff->ft != yed_get_ft("C")) {
        return;
    }
    if (buff->path == NULL) {
        return;
    }
    if ((ext = get_path_ext(buff->path)) == NULL) {
        return;
    }

    if (strcmp(ext, "h") == 0) {
        if (kwd_scan(buff)) {
            yed_buffer_set_ft(buff, yed_get_ft("C++"));
        }
    } else if (strcmp(ext, "cpp") == 0 ||
               strcmp(ext, "cxx") == 0 ||
               strcmp(ext, "hpp") == 0 ||
               strcmp(ext, "hxx") == 0) {

        yed_buffer_set_ft(buff, yed_get_ft("C++"));
    }
}

void maybe_change_ft_event(yed_event *event) {
    if (event->buffer) {
        maybe_change_ft(event->buffer);
    }
}
