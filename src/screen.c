#include "screen.h"

void yed_init_screen(void) {
    ys->output_buffer = array_make_with_cap(char, 4 * ys->term_cols * ys->term_rows);

    ys->screen_update = &ys->screen1;
    ys->screen_render = &ys->screen2;

    ys->screen_update->opacity = DEFAULT_FAKE_OPACITY;

    yed_resize_screen();
}

void yed_resize_screen(void) {
    int              n_cells;
    int              n_bytes;
    yed_screen_cell *cell;
    int              i;

    n_cells = ys->term_rows * ys->term_cols;
    n_bytes = n_cells * sizeof(yed_screen_cell);

    ys->screen_update->cells = realloc(ys->screen_update->cells, n_bytes);
    ys->screen_render->cells = realloc(ys->screen_render->cells, n_bytes);

    memset(ys->screen_update->cells, 0, n_bytes);
    memset(ys->screen_render->cells, 0, n_bytes);

    cell = ys->screen_render->cells;
    for (i = 0; i < n_cells; i += 1) {
        cell->dirty  = 1;
        cell        += 1;
    }
}

void yed_clear_screen(void) {
    printf(TERM_RESET TERM_CURSOR_HOME TERM_CLEAR_SCREEN);
    yed_resize_screen();
}

__attribute__((always_inline))
static inline void set_cellp(yed_screen_cell *cell, yed_glyph g) {
    cell->attrs = ys->screen_update->cur_attrs;
    cell->glyph = g;
}

__attribute__((always_inline))
static inline void set_cellp_combine(yed_screen_cell *cell, yed_glyph g) {
    cell->attrs.flags &= ~(ATTR_BOLD | ATTR_UNDERLINE | ATTR_ITALIC | ATTR_INVERSE);

    yed_combine_attrs(&cell->attrs, &ys->screen_update->cur_attrs);
    cell->glyph = g;
}

__attribute__((always_inline))
static inline yed_screen_cell *get_cell(int row, int col) {
    yed_screen_cell *cell;

    cell = ys->screen_update->cells + ((row - 1) * ys->term_cols) + (col - 1);

    return cell;
}

__attribute__((always_inline))
static inline void set_cell(int row, int col, yed_glyph g) {
    yed_screen_cell *cell;

    cell = ys->screen_update->cells + ((row - 1) * ys->term_cols) + (col - 1);

    set_cellp(cell, g);
}

__attribute__((always_inline))
static inline void set_cell_combine(int row, int col, yed_glyph g) {
    yed_screen_cell *cell;

    cell = ys->screen_update->cells + ((row - 1) * ys->term_cols) + (col - 1);

    set_cellp_combine(cell, g);
}

static void write_welcome(void) {
    int   i, j, n_oct_lines, oct_width, l, col, row;
    char *oct[] = {
"                                                           \n",
"                                                        //(\n",
"                                                      //   \n",
"                   This is Your Editor.              %/    \n",
"                                                     //    \n",
"                                                    //,    \n",
"                               %                    /#     \n",
"                               */#(     //***/*/*  /*/     \n",
"                       &(//////% ,/**//(       (//*,       \n",
"                      //////////*(              /*/*       \n",
"                     (//(///*////*(           ***/ *(      \n",
"                    ((///////*////#        **/*(  (/(      \n",
"                    #////////*///(#   /(****#    /**       \n",
"                    (///**/*****%##***(#/       (*/(       \n",
"                    ./////***(#%/%((,         (/*/         \n",
"                     %///***(((%/#*       /*/**#           \n",
"                     (#//***///(%*(///**/(((*              \n",
"                       //*//*//#(////((%#(%(%              \n",
"                  %&((//**/**************/**/***(          \n",
"              *(///###///***********//(((###///***/#       \n",
"           /////(((///((//**/#/***//*//        /#****(     \n",
"        //(//##(///#    #/*/( /#////#//*/         (****#   \n",
"      //((#%((((//      (///    (//*( %(/*&         ****   \n",
"     (/((&  #(/(.       ((//    (/**/   #/*(         ***/  \n",
"     /((#   #///       (///*   ,*/**(    (/*#        ****  \n",
"     ((%    (/(*      ////(    (/*/*      (/*(      (**/   \n",
"     ((#    (((      (///      ///*        ***     (***    \n",
"     #((    ((/%    ((//      (///         //*/    /**.    \n",
"     .(((    ((/   #(/(      */*(          ///     ***     \n",
"      ((/    #((( (((/(     ///(           */*     **(     \n",
"      #/%     ((( (//*      */*            //*     ***/    \n",
"      (/       /((#(/*      /**            (/*(     .***(  \n",
"     %(%       //(,////     //*/           (*/*         (/(\n",
"    #((         // #(/*      */*(           #**/           \n",
"  (#(&          (/  ////      (//(*           *//&         \n",
"  .             //   //*/       #//*#(&         (**(/      \n",
"               ,/(    /**           #//*/#           //%   \n",
"               //     */*(              //*(/          **  \n",
"              //*      (//                *//#             \n",
"             //         //(                 *//            \n",
"           *(/          ///                  *(            \n",
"          .             #//                  /(            \n",
"                        #//                  ((            \n",
"                        (#                   /(            \n",
"                        (                    (             \n",
"                       ((                                  \n"
    };

    l = 1;

    n_oct_lines = sizeof(oct) / sizeof(char*);
    oct_width   = strlen(oct[0]);

    for (i = 0; i < n_oct_lines; i += 1) {
        row = l + i;

        if (row == ys->term_rows - 1) {
            break;
        }

        col = ((ys->term_cols / 2) - (oct_width / 2));
        for (j = 0; j < oct_width - 1; j += 1) {

            if (col+j > ys->term_cols) {
                break;
            }

            if (col+j < 1) {
                continue;
            }

            set_cell(l + i, (ys->term_cols / 2) - (oct_width / 2) + j, G(oct[i][j]));
        }
    }
}

void yed_draw_background(void) {
    int              n_cells;
    yed_glyph        space;
    yed_screen_cell *cell;
    int              i;

    yed_set_attr(yed_active_style_get_inactive());

    n_cells = ys->term_rows * ys->term_cols;
    space   = G(' ');
    cell    = ys->screen_update->cells;

    for (i = 0; i < n_cells; i += 1) {
        set_cellp(cell, space);
        cell += 1;
    }

    write_welcome();
}

void yed_diff_and_swap_screens(void) {
    int              n_cells;
    yed_screen_cell *ucell;
    yed_screen_cell *rcell;
    int              i;
    int              dirty;

    n_cells = ys->term_rows * ys->term_cols;
    ucell   = ys->screen_update->cells;
    rcell   = ys->screen_render->cells;

    for (i = 0; i < n_cells; i += 1) {
        dirty =    (rcell->glyph.data != ucell->glyph.data)
                || (!ATTRS_EQ(rcell->attrs, ucell->attrs));

        *rcell       = *ucell;
        rcell->dirty = dirty;

        ucell += 1;
        rcell += 1;
    }
}

void yed_render_screen(void) {
    int              screen_dirty;
    yed_screen_cell *cell;
    int              row;
    int              col;
    char             buff[512];
    int              cursor_x;
    int              cursor_y;
    int              width;
    int              total_written;
    int              n;

#define WR(s, n) array_push_n(ys->output_buffer, s, n)

    array_clear(ys->output_buffer);

    if (yed_var_is_truthy("screen-update-sync")) {
        WR("\e[?2026h", strlen("\e[?2026h"));
    }

    screen_dirty = 0;

    /* Screen is "dirty" if the cursor has moved. */
    if (ys->interactive_command != NULL) {
        if (ys->screen_render->cur_x != ys->cmd_cursor_x
        ||  ys->screen_render->cur_y != ys->term_rows) {
            screen_dirty = 1;
        }
    } else if (ys->active_frame != NULL) {
        if (ys->screen_render->cur_x != ys->active_frame->cur_x
        ||  ys->screen_render->cur_y != ys->active_frame->cur_y) {
            screen_dirty = 1;
        }
    }

    WR(TERM_CURSOR_HIDE, strlen(TERM_CURSOR_HIDE));

    WR("\e[H", strlen("\e[H"));
    ys->screen_render->cur_x = ys->screen_render->cur_y = 1;

    ys->screen_render->cur_attrs = ZERO_ATTR;
    WR(TERM_RESET, strlen(TERM_RESET));

    cell = ys->screen_render->cells;

    for (row = 1; row <= ys->term_rows; row += 1) {
        for (col = 1; col <= ys->term_cols; col += 1) {
            if (cell->dirty && cell->glyph.data) {
                screen_dirty = 1;

                if (ys->screen_render->cur_y != row
                &&  ys->screen_render->cur_x != col) {
                    snprintf(buff, sizeof(buff), "\e[%d;%dH", row, col);
                    WR(buff, strlen(buff));
                    ys->screen_render->cur_y = row;
                    ys->screen_render->cur_x = col;
                } else if (ys->screen_render->cur_y != row) {
                    snprintf(buff, sizeof(buff), "\e[%dd", row);
                    WR(buff, strlen(buff));
                    ys->screen_render->cur_y = row;
                } else if (ys->screen_render->cur_x != col) {
                    snprintf(buff, sizeof(buff), "\e[%dG", col);
                    WR(buff, strlen(buff));
                    ys->screen_render->cur_x = col;
                }

                if (!ATTRS_EQ(cell->attrs, ys->screen_render->cur_attrs)) {
                    yed_get_attr_str(cell->attrs, buff);
                    WR(buff, strlen(buff));
                    ys->screen_render->cur_attrs = cell->attrs;
                }

                WR(&cell->glyph.c, yed_get_glyph_len(cell->glyph));

                width = yed_get_glyph_width(cell->glyph);
                ys->screen_render->cur_x += width;
            }

            cell->dirty = 0;
            cell += 1;
        }
    }

    if (ys->interactive_command != NULL) {
        cursor_y = ys->term_rows;
        cursor_x = ys->cmd_cursor_x;
        WR(TERM_CURSOR_SHOW, strlen(TERM_CURSOR_SHOW));
    } else if (ys->active_frame != NULL) {
        cursor_y = ys->active_frame->cur_y;
        cursor_x = ys->active_frame->cur_x;
        WR(TERM_CURSOR_SHOW, strlen(TERM_CURSOR_SHOW));
    } else {
        cursor_y = cursor_x = 1;
    }

    ys->screen_render->cur_x = cursor_x;
    ys->screen_render->cur_y = cursor_y;

    if (screen_dirty) {
        snprintf(buff, sizeof(buff), "\e[%d;%dH", cursor_y, cursor_x);
        WR(buff, strlen(buff));

        if (yed_var_is_truthy("screen-update-sync")) {
            WR("\e[?2026l", strlen("\e[?2026l"));
        }

        total_written = 0;
        while (total_written < array_len(ys->output_buffer)) {
            n = write(1, array_data(ys->output_buffer) + total_written, array_len(ys->output_buffer) - total_written);
            ASSERT(n > 0, "failed to write output");
            total_written += n;
        }
    }

#undef WR
}

__attribute__((always_inline))
static inline void screen_print_n(const char *s, int n, int combine) {
    const char      *end;
    yed_attrs        save_attrs;
    yed_glyph       *g;
    int              len;
    int              width;
    yed_screen_cell *cellp;
    yed_glyph        new_g;
    float            opacity;
    int              transparent;
    int              i;
    int              br;
    int              bg;
    int              bb;
    int              fr;
    int              fg;
    int              fb;

    end = s + n;

    while (s < end) {
        if (unlikely(   ys->screen_update->cur_y > ys->term_rows
                     || ys->screen_update->cur_x > ys->term_cols)) {

            break;
        }

        g     = (yed_glyph*)(void*)s;
        len   = yed_get_glyph_len(*g);
        width = yed_get_glyph_width(*g);
        new_g = G(0);
        for (i = 0; i < len; i += 1) { new_g.bytes[i] = g->bytes[i]; }

        opacity     = ys->screen_update->opacity;
        transparent = opacity < 1.0 && opacity > 0 && combine && ATTR_BG_KIND(ys->screen_update->cur_attrs.flags) == ATTR_KIND_RGB;

        if (transparent) {
            cellp                        = get_cell(ys->screen_update->cur_y, ys->screen_update->cur_x);
            save_attrs                   = ys->screen_update->cur_attrs;
            ys->screen_update->cur_attrs = save_attrs;
            br                           = (int)(opacity * RGB_32_r(save_attrs.bg));
            bg                           = (int)(opacity * RGB_32_g(save_attrs.bg));
            bb                           = (int)(opacity * RGB_32_b(save_attrs.bg));

            if (new_g.c == ' ') {
                new_g = cellp->glyph;
                fr                              = (int)((1.0 - opacity) * RGB_32_r(cellp->attrs.fg));
                fg                              = (int)((1.0 - opacity) * RGB_32_g(cellp->attrs.fg));
                fb                              = (int)((1.0 - opacity) * RGB_32_b(cellp->attrs.fg));
                ys->screen_update->cur_attrs.fg = RGB_32(br + fr, bg + fg, bb + fb);
            }

            fr                              = (int)((1.0 - opacity) * RGB_32_r(cellp->attrs.bg));
            fg                              = (int)((1.0 - opacity) * RGB_32_g(cellp->attrs.bg));
            fb                              = (int)((1.0 - opacity) * RGB_32_b(cellp->attrs.bg));
            ys->screen_update->cur_attrs.bg = RGB_32(br + fr, bg + fg, bb + fb);
        }

        for (i = 0; i < width; i += 1) {
            if (combine) {
                set_cell_combine(ys->screen_update->cur_y, ys->screen_update->cur_x + i, i == 0 ? new_g : G(0));
            } else {
                set_cell(ys->screen_update->cur_y, ys->screen_update->cur_x + i, i == 0 ? new_g : G(0));
            }
        }

        if (transparent) {
            ys->screen_update->cur_attrs = save_attrs;
        }

        ys->screen_update->cur_x += width;

        s = s + len;
    }
}

void yed_screen_print(const char *s)               { if (!s) { return; } yed_screen_print_n(s, strlen(s));      }
void yed_screen_print_n(const char *s, int n)      { if (!s) { return; } screen_print_n(s, n, 0);               }
void yed_screen_print_over(const char *s)          { if (!s) { return; } yed_screen_print_n_over(s, strlen(s)); }
void yed_screen_print_n_over(const char *s, int n) { if (!s) { return; } screen_print_n(s, n, 1);               }

void yed_screen_print_single_cell_glyph(yed_glyph g) {
    set_cell(ys->screen_update->cur_y, ys->screen_update->cur_x, g);
    ys->screen_update->cur_x += 1;
}

void yed_screen_print_single_cell_glyph_over(yed_glyph g) {
    set_cell_combine(ys->screen_update->cur_y, ys->screen_update->cur_x, g);
    ys->screen_update->cur_x += 1;
}
