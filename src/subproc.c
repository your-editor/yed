char * yed_run_subproc(char *cmd) {
    FILE    *stream;
    array_t  out;
    char     c;

    stream = popen(cmd, "r");

    if (stream == NULL)    { return NULL; }

    out = array_make(char);

    c = 0;
    while ((c = fgetc(stream)) >= 0) {
        if (c == '\r') {
            continue;
        }
        array_push(out, c);
    }

    if (array_len(out)
    &&  *(char*)array_last(out) == '\n') {
        array_pop(out);
    }

    array_zero_term(out);

    pclose(stream);

    return array_data(out);
}
