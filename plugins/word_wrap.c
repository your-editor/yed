#include <yed/plugin.h>

void remove_preceding_whitespace(yed_buffer *buff, int row);
void remove_trailing_whitespace(yed_buffer *buff, int row);
int is_line_all_whitespace(yed_line *line);
int combine_line_with_next(yed_buffer *buff, yed_line *line, int row, int max_cols);
int absorb_first_word_of_next_line(yed_buffer *buff, yed_line *line, int row, int max_cols);
void word_wrap(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    yed_plugin_set_command(self, "word-wrap", word_wrap);

    return 0;
}

/* Determine if a line is the beginning of a paragraph or not. */
int is_beginning_of_paragraph(yed_buffer *buff, int row) {
    yed_line *line;
    yed_glyph *git;
    
    line = yed_buff_get_line(buff, row - 1);
    if(!line) {
        /* The first line is the beginning of a paragraph */
        return 1;
    }
    
    /* `row` is the beginning of a paragraph if the previous
       row contains no glyphs or all whitespace. */
    yed_line_glyph_traverse(*line, git) {
        if(git->c != ' ') return 0;
    }
    
    return 1;
}

/* This function is used to remove whitespace that's at the end of a line.
   We only call this if we delete words from a line, and the resulting
   line happens to end with whitespace. */
void remove_trailing_whitespace(yed_buffer *buff, int row) {
    yed_line *line;
    yed_glyph *git;
    int num_glyphs, i;
    
    line = yed_buff_get_line(buff, row);
    
    num_glyphs = 0;
    yed_line_glyph_traverse(*line, git) {
        if(git->c != ' ') {
            num_glyphs = 0;
            continue;
        } else {
            num_glyphs++;
        }
    }
    
    for(i = 0; i < num_glyphs; i++) {
        yed_pop_from_line(buff, row);
    }
}

/* This function is used to remove whitespace that begins a line.
   We only call this if we delete words from a line, and the resulting
   line happens to begin with whitespace. */
void remove_preceding_whitespace(yed_buffer *buff, int row) {
    yed_line *line;
    yed_glyph *git;
    int num_glyphs, i;
    
    line = yed_buff_get_line(buff, row);
    
    num_glyphs = 0;
    yed_line_glyph_traverse(*line, git) {
        if(git->c == ' ') num_glyphs++;
        else break;
    }
    
    for(i = 0; i < num_glyphs; i++) {
        yed_delete_from_line(buff, row, 1);
    }
}

/* Duh */
int is_line_all_whitespace(yed_line *line) {
    yed_glyph *git;
    yed_line_glyph_traverse(*line, git) {
        if(git->c != ' ') return 0;
    }
    return 1;
}

/* Continuously absorbs the first word of the next line, until we can't anymore. 
   Returns the number of lines that we deleted. */
int combine_line_with_next(yed_buffer *buff, yed_line *line, int row, int max_cols) {
    int retval, num_deleted;
    
    retval = 1;
    num_deleted = 0;
    while(retval == 1) {
        retval = absorb_first_word_of_next_line(buff, line, row, max_cols);
        if(retval == -1) {
            num_deleted++;
            retval = 1;
        }
    }
    
    return num_deleted;
}

/* Combine the current line with the first word of the next line, making it no more than max_cols in width.
   Returns the number of words it was able to absorb (1 or 0). */
int absorb_first_word_of_next_line(yed_buffer *buff, yed_line *line, int row, int max_cols) {
    yed_glyph *git;
    int i, num_glyphs, first_word_width, first_word_visual_width;
    char seen_non_whitespace, preceding_whitespace;
    yed_line *next_line;
    
    next_line = yed_buff_get_line(buff, row + 1);
    if(!next_line) {
        /* It's possible for this line to no longer exist. It could have
           been consumed in an earlier call to this function. */
        return 0;
    }
    
    /* Start by inspecting the next line. In order to do anything whatsoever,
       we need `line->visual_width` plus the visual width of the first word on
       the next line to be less than `max_cols`. */
    first_word_visual_width = 0;
    first_word_width = 0;
    seen_non_whitespace = 0;
    preceding_whitespace = 0;
    yed_line_glyph_traverse(*next_line, git) {
        if(git->c != ' ') {
            seen_non_whitespace = 1;
        }
        if(seen_non_whitespace && (git->c == ' ')) {
            break;
        } else if(git->c == ' ') {
            preceding_whitespace = 1;
        }
        first_word_visual_width += yed_get_glyph_width(*git);
        first_word_width++;
        
        /* Bail out if we've already exceeded our limit */
        if(max_cols < first_word_visual_width + line->visual_width) {
            break;
        }
    }
    
    /* If there was no preceding whitespace, we'll have to add a space. */
    if(!preceding_whitespace) {
        first_word_visual_width++;
    }
    
    /* We'll only combine if the first word (incl. preceding whitespace) can fit, AND
       if we've seen any non-whitespace characters at all. */
    if((max_cols >= first_word_visual_width + line->visual_width) && seen_non_whitespace) {
        num_glyphs = 0;
        
        /* If there was no preceding whitespace, we'll need to add some */
        if(!preceding_whitespace) {
            yed_append_to_line(buff, row, G(' '));
        }
        
        /* Copy the first word from the next line to the current one */
        yed_line_glyph_traverse(*next_line, git) {
            if(num_glyphs >= first_word_width) {
                break;
            }
            yed_append_to_line(buff, row, *git);
            num_glyphs++;
        }
        
        /* If we've removed all characters from the next line, delete it.
            Return -1 if we deleted a line. */
        if(next_line->n_glyphs == num_glyphs) {
            yed_buff_delete_line(buff, row + 1);
            return -1;
        }
        
        /* Remove the glyphs that we just copied. Make sure the resulting 
            line doesn't start with whitespace.  Return 1 if we combined
            a line, but didn't delete one. */
        for(i = 0; i < num_glyphs; i++) {
            yed_delete_from_line(buff, row + 1, 1);
        }
        remove_preceding_whitespace(buff, row + 1);
        return 1;
    }
    
    /* Return 0 if we didn't combine any lines */
    return 0;
}

/* Create a new line, find the last word that can fit in the limit, and copy
   the rest of the line to the new line. Returns 1 if it created a new line,
   and 0 otherwise. */
int split_line(yed_buffer *buff, int row, int max_cols) {
    yed_glyph  *git, *prev_git, *tmp_git;
    int i, idx, col, num_glyphs_to_pop, prev_word_col;
    yed_line *line;
    
    col = 1;
    prev_git = NULL;
    prev_word_col = 0;
    
    line = yed_buff_get_line(buff, row);
    yed_line_glyph_traverse(*line, git) {
        
        if((prev_git && (prev_git->c == ' ')) && (git->c != ' ')) {
            /* prev_word_col will never be set to the first column. */
            prev_word_col = col;
        }
        
        /* If the current column is over the limit */
        if((col > max_cols) && prev_word_col) {
            /* Break on the last whitespace that we found */
            yed_buff_insert_line(buff, row + 1);
            if(git->c != ' ') {
                /* If we're in a word, split on the start of the last word we've seen */
                idx = yed_line_col_to_idx(line, prev_word_col);
            } else {
                /* If we're on whitespace, just split here. */
                idx = yed_line_col_to_idx(line, col);
            }
            num_glyphs_to_pop = 0;
            yed_line_glyph_traverse_from(*line, tmp_git, idx) {
                yed_append_to_line(buff, row + 1, *tmp_git);
                num_glyphs_to_pop++;
            }
            for(i = 0; i < num_glyphs_to_pop; i++) {
                yed_pop_from_line(buff, row);
            }
            remove_trailing_whitespace(buff, row);
            
            /* We have to break here, because the glyph pointer is now invalidated. */
            return 1;
        }
        
        prev_git = git;
        col += yed_get_glyph_width(*git);
    }
    
    return 0;
}

void word_wrap(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    int         r1, c1, r2, c2, row, max_cols, n_lines, num_deleted;
    yed_line   *line;

    if (n_args != 1) {
        yed_cerr("expected 1 argument, a maximum number of columns per line");
        return;
    }
    sscanf(args[0], "%d", &max_cols);

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
    if (!(buff->has_selection)) {
        r1 = 1;
        c1 = 1;
        n_lines = yed_buff_n_lines(buff);
        r2 = n_lines ? n_lines : 1;
        c2 = 1;
    } else {
        yed_range_sorted_points(&buff->selection, &r1, &c1, &r2, &c2);
    }

    yed_start_undo_record(frame, buff);

    for (row = r1; row <= r2; row += 1) {
        line = yed_buff_get_line(buff, row);
        
        if(!line || is_line_all_whitespace(line)) {
            continue;
        }
        if(!is_beginning_of_paragraph(buff, row)) {
            remove_preceding_whitespace(buff, row);
        }
        remove_trailing_whitespace(buff, row);
        
        if(line->visual_width > max_cols) {
            if(split_line(buff, row, max_cols)) {
                r2++;
            }
        } else if((line->visual_width < max_cols) && (row != r2)) {
            num_deleted = combine_line_with_next(buff, line, row, max_cols);
            r2 -= num_deleted;
        }
    }
    
    yed_set_cursor_within_frame(frame, 1, r2);
    yed_end_undo_record(frame, buff);
    frame->dirty = 1;

    YEXE("select-off");
}
