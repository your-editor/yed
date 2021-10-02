char * yed_run_subproc(char *cmd, int *output_len, int *status) {
    FILE    *stream;
    array_t  out;
    int      c;
    int      w;

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

    if (output_len != NULL) {
        *output_len = array_len(out);
    }

    w = pclose(stream);

    if (status != NULL) {
        *status = WIFEXITED(w) ? WEXITSTATUS(w) : -1;
    }

    array_zero_term(out);

    return array_data(out);
}

int yed_read_subproc_into_buffer(char *cmd, yed_buffer *buff, int *exit_status) {
    FILE *stream;
    int   status;
    int   w;

    stream = popen(cmd, "r");
    if (stream == NULL) {
        status = errno;
        errno  = 0;
        return status;
    }

    status = yed_fill_buff_from_file_stream(buff, stream);
    if (status) {
        pclose(stream);
        return -status;
    }

    w = pclose(stream);
    if (w == -1) {
        status = errno;
        errno  = 0;
        return status;
    }

    if (exit_status != NULL) {
        *exit_status = WIFEXITED(w) ? WEXITSTATUS(w) : -1;
    }

    return 0;
}

int _yed_write_buffer_to_subproc(yed_buffer *buff, char *cmd, int *exit_status) {
    FILE     *stream;
    int       status;
    int       n_lines;
    int       row;
    yed_line *line;
    int       w;

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

    w = pclose(stream);
    if (w == -1) {
        status = errno;
        errno  = 0;
        return status;
    }

    if (exit_status != NULL) {
        *exit_status = WIFEXITED(w) ? WEXITSTATUS(w) : -1;
    }

    return 0;
}

int _yed_write_buffer_to_subproc_2(yed_buffer *buff, char *cmd, int *exit_status, char **output) {
    int            status;
    int            fds_to_child[2];
    int            fds_from_child[2];
    pid_t          pid;
    int            n_lines;
    int            row;
    yed_line      *line;
    array_t        out;
    char           read_buff[4096];
    int            n_read;
    int            wait_status;

    status = 0;

    if (buff == NULL) {
        status = EINVAL;
        goto out;
    }

    if (pipe(fds_to_child) == -1) {
        status = errno;
        errno  = 0;
        goto out;
    }
    if (pipe(fds_from_child) == -1) {
        status = errno;
        errno  = 0;
        goto out;
    }

    pid = fork();

    if (pid == -1) {
        status = errno;
        errno  = 0;
        close(fds_to_child[0]);   close(fds_to_child[1]);
        close(fds_from_child[0]); close(fds_from_child[1]);
        goto out;
    }

    if (pid == 0) {
        while ((dup2(fds_to_child[0],   0) == -1) && (errno == EINTR)) {}
        while ((dup2(fds_from_child[1], 1) == -1) && (errno == EINTR)) {}
        close(fds_to_child[0]);   close(fds_to_child[1]);
        close(fds_from_child[0]); close(fds_from_child[1]);
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        exit(99);
    }

    close(fds_to_child[0]);
    close(fds_from_child[1]);

    n_lines = yed_buff_n_lines(buff);
    row     = 1;
    bucket_array_traverse(buff->lines, line) {
        (void)write(fds_to_child[1], array_data(line->chars), array_len(line->chars));
        if (row < n_lines) {
            (void)write(fds_to_child[1], "\n", 1);
        }
        row += 1;
    }
    close(fds_to_child[1]);

    out = array_make(char);
    while ((n_read = read(fds_from_child[0], read_buff, sizeof(read_buff))) > 0) {
        array_push_n(out, read_buff, n_read);
    }

    if (n_read == -1) {
        status = errno;
        errno  = 0;
        close(fds_from_child[0]);
        array_free(out);
        goto out;
    }

    close(fds_from_child[0]);
    array_zero_term(out);
    *output = array_data(out);

    do {
        status = waitpid(pid, &wait_status, 0);
        if (status == -1) {
            status = errno;
            errno  = 0;
            array_free(out);
            *output = NULL;
            goto out;
        }
        status = 0;
    } while (!WIFEXITED(wait_status));

    if (exit_status != NULL) {
        *exit_status = WEXITSTATUS(wait_status);
    }

out:;
    return status;
}

int yed_write_buffer_to_subproc(yed_buffer *buff, char *cmd, int *exit_status, char **output) {
    if (output == NULL) {
        return _yed_write_buffer_to_subproc(buff, cmd, exit_status);
    }

    return _yed_write_buffer_to_subproc_2(buff, cmd, exit_status, output);
}

int yed_start_read_subproc_into_buffer_nb(char *cmd, yed_buffer *buff, yed_nb_subproc_t *nb_subproc) {
    int   status;
    int   fds[2];
    pid_t pid;

    status = 0;

    if (buff == NULL || nb_subproc == NULL) {
        status = EINVAL;
        goto out;
    }

    memset(nb_subproc, 0, sizeof(*nb_subproc));

    if (pipe(fds) == -1) {
        status = errno;
        errno  = 0;
        goto out;
    }

    pid = fork();

    if (pid == -1) {
        status = errno;
        errno  = 0;
        close(fds[0]);
        close(fds[1]);
        goto out;
    }

    if (pid == 0) {
        while ((dup2(fds[1], 1) == -1) && (errno == EINTR)) {}
        close(fds[1]);
        close(fds[0]);
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        exit(99);
    }

    close(fds[1]);

    if (fcntl(fds[0], F_SETFL, O_NONBLOCK) == -1) {
        status = errno;
        errno  = 0;
        close(fds[0]);
        goto out;
    }

    yed_buff_clear_no_undo(buff);

    nb_subproc->pid         = pid;
    nb_subproc->fd          = fds[0];
    nb_subproc->buffer      = buff;
    nb_subproc->exit_status = 0;
    nb_subproc->err         = 0;

out:;
    return status;
}

int yed_read_subproc_into_buffer_nb(yed_nb_subproc_t *nb_subproc) {
    int        status;
    int        wait_status;
    int        exited;
    int        last_row;
    int        n_read;
    char       buff[4096];
    yed_glyph *g_start;
    yed_glyph *g;
    yed_line  *last_line;

    status = 0;

    if (waitpid(nb_subproc->pid, &wait_status, WNOHANG) == -1) {
        status          = 0;
        nb_subproc->err = errno;
        errno           = 0;
        goto out;
    }

    exited = WIFEXITED(wait_status);

    last_row = yed_buff_n_lines(nb_subproc->buffer);
    while ((n_read = read(nb_subproc->fd, buff, sizeof(buff))) > 0) {

        g_start = (yed_glyph*)(&buff[0]);
        for (g = g_start; ((void*)g) - ((void*)g_start) < n_read;) {

            if (g->c == '\r') { /* Ignore */
            } else if (g->c == '\n') {
                last_row = yed_buffer_add_line_no_undo(nb_subproc->buffer);
            } else {
                yed_append_to_line_no_undo(nb_subproc->buffer, last_row, *g);
            }

            g = ((void*)g) + yed_get_glyph_len(*g);
        }
    }

    if (n_read < 0) {
        if (errno == EAGAIN) {
            errno  = 0;
            status = 1;
            goto out;
        } else {
            status          = 0;
            nb_subproc->err = errno;
            errno           = 0;
            goto out;
        }
    }

    if (exited) {
        if (yed_buff_n_lines(nb_subproc->buffer) > 1) {
            last_line = bucket_array_last(nb_subproc->buffer->lines);
            if (last_line->visual_width == 0) {
                yed_buff_delete_line_no_undo(nb_subproc->buffer, yed_buff_n_lines(nb_subproc->buffer));
            }
        }
        nb_subproc->exit_status = WEXITSTATUS(wait_status);
        status = 0;
    } else {
        status = 1;
    }

out:;
    if (status == 0) {
        close(nb_subproc->fd);
        nb_subproc->fd = -1;
    }

    return status;
}
