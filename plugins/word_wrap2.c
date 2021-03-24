#include <yed/plugin.h>

void word_wrap2(int n_args, char **args);

int yed_plugin_boot(yed_plugin *self) {
        YED_PLUG_VERSION_CHECK();
        yed_plugin_set_command(self, "word-wrap2", word_wrap2);
        return 0;
}

char is_line_whitespace(yed_buffer *buff, int row) {
    yed_line    *line;
    yed_glyph *git;
    
    line = yed_buff_get_line(buff, row);
    if(!line) {
        return 0;
    }
    yed_line_glyph_traverse(*line, git) {
        if(git->c != ' ') {
            return 0;
        }
    }
    return 1;
}

/* This function removes trailing whitespace from a line. */
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

/* This function is used to remove whitespace that begins a line. */
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

char split_line(yed_buffer *buff, int row, yed_glyph *start_git, int start_col, char end_of_selection) {
    yed_line  *line, *next_line;
    yed_glyph *git;
    int idx, glyphs_to_delete, i, col;
    char created_line;
    
    line = yed_buff_get_line(buff, row);
    idx = ((void*)start_git) - array_data(line->chars);
    next_line = yed_buff_get_line(buff, row + 1);
    
    /* Create a line if necessary */
    created_line = 0;
    if(!next_line || end_of_selection || is_line_whitespace(buff, row + 1)) {
        /* We'll need to create the next line */
        yed_buff_insert_line(buff, row + 1);
        next_line = yed_buff_get_line(buff, row + 1);
        created_line = 1;
    }
    
    /* Loop over glyphs and prepend them onto the next line */
    glyphs_to_delete = 0;
    col = 1;
    yed_line_glyph_traverse_from(*line, git, idx) {
        yed_insert_into_line(buff, row + 1, col, *git);
        glyphs_to_delete++;
        col += yed_get_glyph_width(*git);
    }
    
    /* We need a space after the words that we just inserted, otherwise they're
       going to merge with the words that're already on the line */
    yed_buff_insert_string(buff, " ", row + 1, col);
    
    /* Delete the glyphs that we moved to the next line */
    for(i = 0; i < glyphs_to_delete; i++) {
        yed_pop_from_line(buff, row);
    }
    
    if(created_line) {
        return 1;
    }
    return 0;
}

void word_wrap2(int n_args, char **args) {
    yed_frame  *frame;
    yed_buffer *buff;
    yed_glyph  *git, *prev_git, *word_git;
    yed_line   *line;
    int        r1, c1,
               r2, c2, 
               row, max_cols, word_col,
               num_line_cols;
    char       begins_par, in_word, end_of_selection;
    
    if (n_args != 1) {
        yed_cerr("expected 1 argument, a maximum number of columns per line");
        return;
    }
    sscanf(args[0], "%d", &max_cols);
    
    /* Get the frame and buffer */
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
    if (!buff->has_selection) {
    }
    
    yed_range_sorted_points(&buff->selection, &r1, &c1, &r2, &c2);
    yed_start_undo_record(frame, buff);
    
    /* Iterate over the words until they've hit the maximum number of columns.
       Once they've done that, start pushing words onto the next line. */
    end_of_selection = 0;
    for (row = r1; row <= r2; row += 1) {
        line = yed_buff_get_line(buff, row);
        
        if(row == r2) {
            end_of_selection = 1;
        }
        
        if(is_line_whitespace(buff, row)) {
            continue;
        }
        
        begins_par = 0;
        if((row == r1) || is_line_whitespace(buff, row - 1)) {
            /* A line begins a paragraph if any are true:
               1. It's the first line in a selection.
               2. The previous line doesn't exist.
               3. The previous line is all whitespace. */
            begins_par = 1;
        }
        
        if(!begins_par) {
            /* If the line doesn't start a paragraph, remove its
               preceding whitespace */
            remove_preceding_whitespace(buff, row);
        }
        remove_trailing_whitespace(buff, row);
        
        /* Loop over the glyphs on the line until we get to the maximum number of columns.
           Remember various things, like where the start of the last word is, so that we
           don't have to backtrack. */
        num_line_cols = 0;
        word_col = 0;
        word_git = NULL;
        prev_git = NULL;
        in_word = 0;
        yed_line_glyph_traverse(*line, git) {
            
            /* This block figures out if we're currently in a word,
               when the last word beginning occurred, and how many
               columns are in each word */
            if(git->c != ' ') {
                in_word = 1;
                if(!prev_git || (prev_git && (prev_git->c == ' '))) {
                    /* This is the start of a word */
                    word_git = git;
                    word_col = num_line_cols + 1;
                }
            } else {
                in_word = 0;
                if(prev_git && (prev_git->c != ' ')) {
                    /* This is the end of a word */
                }
            }
            
            if((num_line_cols + yed_get_glyph_width(*git)) > max_cols) {
                /* If we've reached the limit on the number of columns */
                if(!in_word) {
                    word_git = git;
                    word_col = num_line_cols;
                }
                if(split_line(buff, row, word_git, word_col, end_of_selection)) {
                    r2++;
                }
                break;
            }
            num_line_cols += yed_get_glyph_width(*git);
            prev_git = git;
        }
        
        remove_trailing_whitespace(buff, row);
    }
    
    //yed_set_cursor_within_frame(frame, save_col, frame->cursor_line);
    yed_end_undo_record(frame, buff);
    frame->dirty = 1;
    
    YEXE("select-off");
}
