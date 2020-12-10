struct popen2 {
    pid_t child_pid;
    int   from_child,
          to_child;
};

static int popen2(const char *cmdline, struct popen2 *childinfo) {
    pid_t p;
    int   pipe_stdin[2], pipe_stdout[2];

    if (pipe(pipe_stdin)) {
        return -1;
    }
    if (pipe(pipe_stdout)) {
        return -1;
    }

    p = fork();

    /* Did fork fail? */
    if (p < 0) { return p; }

    if (p == 0) { /* child */
        close(pipe_stdin[1]);
        dup2(pipe_stdin[0], 0);
        close(pipe_stdout[0]);
        dup2(pipe_stdout[1], 1);
        execl("/bin/sh", "sh", "-c", cmdline, NULL);
        exit(99);
    }

    childinfo->child_pid  = p;
    childinfo->to_child   = pipe_stdin[1];
    childinfo->from_child = pipe_stdout[0];

    return 0;
}

char * yed_run_subproc(char *cmd, int *output_len, int *status) {
    FILE    *stream;
    array_t  out;
    char     c;

    stream = popen(cmd, "r");

    if (stream == NULL)    { return NULL; }

    out = array_make(char);

    c = 0;
    while ((c = fgetc(stream)) != EOF) {
        if (c == '\r') {
            continue;
        }
        array_push(out, c);
    }

    if (array_len(out)
    &&  *(char*)array_last(out) == '\n') {
        array_pop(out);
    }

    *output_len = array_len(out);
    *status     = pclose(stream);

    array_zero_term(out);

    return array_data(out);
}

int yed_read_subproc_into_buffer(char *cmd, yed_buffer *buff, int *exit_status) {
    FILE *stream;
    int   status;

    stream = popen(cmd, "r");
    if (stream == NULL) {
        status = errno;
        errno  = 0;
        return status;
    }

    status = yed_fill_buff_from_file_stream(buff, stream);
    if (status) { return -status; }

    status = pclose(stream);
    if (status == -1) {
        status = errno;
        errno  = 0;
        return status;
    }

    if (exit_status != NULL) {
        *exit_status = status;
    }

    return 0;
}

int _yed_write_buffer_to_subproc(yed_buffer *buff, char *cmd, int *exit_status) {
    FILE     *stream;
    int       status;
    int       n_lines;
    int       row;
    yed_line *line;

    stream = popen(cmd, "w");
    if (stream == NULL) {
        status = errno;
        errno  = 0;
        return status;
    }

    n_lines = yed_buff_n_lines(buff);
    row     = 1;
    bucket_array_traverse(buff->lines, line) {
        fwrite(array_data(line->chars), 1, array_len(line->chars), stream);
        if (row < n_lines) {
            fwrite("\n", 1, 1, stream);
        }
        row += 1;
    }

    status = pclose(stream);
    if (status == -1) {
        status = errno;
        errno  = 0;
        return status;
    }

    if (exit_status != NULL) {
        *exit_status = status;
    }

    return 0;
}

int _yed_write_buffer_to_subproc_2(yed_buffer *buff, char *cmd, int *exit_status, char **output) {
    struct popen2  p2;
    int            status;
    int            n_lines;
    int            row;
    yed_line      *line;
    array_t        out;
    char           read_buff[4096];
    int            n_read;
    int            wait_status;

    status = popen2(cmd, &p2);
    if (status) {
        status = errno;
        errno  = 0;
        return status;
    }

    n_lines = yed_buff_n_lines(buff);
    row     = 1;
    bucket_array_traverse(buff->lines, line) {
        write(p2.to_child, array_data(line->chars), array_len(line->chars));
        if (row < n_lines) {
            write(p2.to_child, "\n", 1);
        }
        row += 1;
    }
    close(p2.to_child);

    out = array_make(char);
    while ((n_read = read(p2.from_child, read_buff, sizeof(read_buff)))) {
        array_push_n(out, read_buff, n_read);
    }
    close(p2.from_child);
    array_zero_term(out);
    *output = array_data(out);

    status = waitpid(p2.child_pid, &wait_status, WEXITED | WSTOPPED);
    if (status == -1) {
        status = errno;
        errno  = 0;
        return status;
    }

    if (exit_status != NULL) {
        *exit_status = WEXITSTATUS(wait_status);
    }

    return 0;
}

int yed_write_buffer_to_subproc(yed_buffer *buff, char *cmd, int *exit_status, char **output) {
    if (output == NULL) {
        return _yed_write_buffer_to_subproc(buff, cmd, exit_status);
    }

    return _yed_write_buffer_to_subproc_2(buff, cmd, exit_status, output);
}
