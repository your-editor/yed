#include "screen.h"

void yed_init_screen(void) {
    ys->screen_update = &ys->screen1;
    ys->screen_render = &ys->screen2;

    yed_resize_screen();
}

void yed_resize_screen(void) {
    int n_cells;
    int n_bytes;

    n_cells = ys->term_rows * ys->term_cols;
    n_bytes = n_cells * sizeof(yed_screen_cell);

    ys->screen_update->cells = realloc(ys->screen_update->cells, n_bytes);
    ys->screen_render->cells = realloc(ys->screen_render->cells, n_bytes);

    memset(ys->screen_update->cells, 0, n_bytes);
    memset(ys->screen_render->cells, 0, n_bytes);
}

void yed_clear_screen(void) {
    printf(TERM_RESET TERM_CURSOR_HOME TERM_CLEAR_SCREEN);
    yed_resize_screen();
}

void yed_set_attr(yed_attrs attr) {
    ys->screen_update->cur_attrs = attr;
}

void yed_reset_attr(void) {
    yed_set_attr(ZERO_ATTR);
}

static void set_cell(int row, int col, yed_glyph g) {
    yed_screen_cell *cell;

    if (row > ys->term_rows || col > ys->term_cols) { return; }

    cell = ys->screen_update->cells + ((row - 1) * ys->term_cols) + (col - 1);

    cell->attrs = ys->screen_update->cur_attrs;
    cell->glyph = g;
}

static void set_cell_combine(int row, int col, yed_glyph g) {
    yed_screen_cell *cell;

    if (row > ys->term_rows || col > ys->term_cols) { return; }

    cell = ys->screen_update->cells + ((row - 1) * ys->term_cols) + (col - 1);

    cell->attrs.flags &= ~(ATTR_BOLD);
    cell->attrs.flags &= ~(ATTR_UNDERLINE);
    cell->attrs.flags &= ~(ATTR_INVERSE);

    yed_combine_attrs(&cell->attrs, &ys->screen_update->cur_attrs);
    cell->glyph = g;
}

static void write_welcome(void) {
    int   i, j, n_oct_lines, oct_width, l;
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
        if (l + i == ys->term_rows - 1) {
            break;
        }
        for (j = 0; j < strlen(oct[i]) - 1; j += 1) {
            set_cell(l + i, (ys->term_cols / 2) - (oct_width / 2) + j, G(oct[i][j]));
        }
    }
}

void yed_draw_background(void) {
    int row;
    int col;

    yed_set_attr(yed_active_style_get_inactive());

    for (row = 1; row <= ys->term_rows; row += 1) {
        for (col = 1; col <= ys->term_cols; col += 1) {
            set_cell(row, col, G(' '));
        }
    }

    write_welcome();
}

void yed_diff_and_swap_screens(void) {
    yed_screen_cell *ucell;
    yed_screen_cell *rcell;
    int              row;
    int              col;
    int              dirty;

    ucell = ys->screen_update->cells;
    rcell = ys->screen_render->cells;

    for (row = 1; row <= ys->term_rows; row += 1) {
        for (col = 1; col <= ys->term_cols; col += 1) {
            dirty = (   rcell->glyph.data != ucell->glyph.data
                     || !yed_attrs_eq(rcell->attrs, ucell->attrs));

            *rcell       = *ucell;
            rcell->dirty = dirty;

            ucell += 1;
            rcell += 1;
        }
    }
}

void yed_render_screen(void) {
    array_t          output_buffer;
    yed_screen_cell *cell;
    int              row;
    int              col;
    char             buff[512];
    int              cursor_x;
    int              cursor_y;
    int              write_ret;

#define WR(s, n) array_push_n(output_buffer, s, n)

    output_buffer = array_make(char);

    WR(TERM_CURSOR_HIDE, strlen(TERM_CURSOR_HIDE));

    ys->screen_render->cur_x = ys->screen_render->cur_y = 1;
    snprintf(buff, sizeof(buff), "%s1%s1%s",
        TERM_CURSOR_MOVE_BEG,
        TERM_CURSOR_MOVE_SEP,
        TERM_CURSOR_MOVE_END);
    WR(buff, strlen(buff));

    ys->screen_render->cur_attrs = ZERO_ATTR;
    WR(TERM_RESET, strlen(TERM_RESET));

    cell = ys->screen_render->cells;

    for (row = 1; row <= ys->term_rows; row += 1) {
        for (col = 1; col <= ys->term_cols; col += 1) {
            if (cell->dirty) {
                if (ys->screen_render->cur_y != row
                ||  ys->screen_render->cur_x != col) {
                    snprintf(buff, sizeof(buff), "%s%d%s%d%s",
                             TERM_CURSOR_MOVE_BEG,
                             row,
                             TERM_CURSOR_MOVE_SEP,
                             col,
                             TERM_CURSOR_MOVE_END);

                    WR(buff, strlen(buff));

                    ys->screen_render->cur_y = row;
                    ys->screen_render->cur_x = col;
                }

                if (!yed_attrs_eq(cell->attrs, ys->screen_render->cur_attrs)) {
                    WR(TERM_RESET, strlen(TERM_RESET));
                    yed_get_attr_str(cell->attrs, buff);
                    WR(buff, strlen(buff));
                    ys->screen_render->cur_attrs = cell->attrs;
                }

                WR(&cell->glyph.c, yed_get_glyph_len(cell->glyph));

                ys->screen_render->cur_x += yed_get_glyph_width(cell->glyph);

                cell->dirty = 0;
            }

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

    snprintf(buff, sizeof(buff), "%s%d%s%d%s",
             TERM_CURSOR_MOVE_BEG,
             cursor_y,
             TERM_CURSOR_MOVE_SEP,
             cursor_x,
             TERM_CURSOR_MOVE_END);

    WR(buff, strlen(buff));

    write_ret = write(1, array_data(output_buffer), array_len(output_buffer));
    (void)write_ret;

    array_free(output_buffer);

#undef WR
}

static void screen_print_n(const char *s, int n, int combine) {
    const char *end;
    yed_glyph  *g;
    int         len;
    yed_glyph   new_g;
    int         width;
    int         i;

    end = s + n;

    while (s < end) {
        g   = (yed_glyph*)(void*)s;
        len = yed_get_glyph_len(*g);

        new_g = G(0);
        memcpy(&new_g, g, len);

        if (combine) {
            set_cell_combine(ys->screen_update->cur_y, ys->screen_update->cur_x, new_g);
        } else {
            set_cell(ys->screen_update->cur_y, ys->screen_update->cur_x, new_g);
        }

        width = yed_get_glyph_width(*g);

        for (i = 1; i < width; i += 1) {
            if (combine) {
                set_cell_combine(ys->screen_update->cur_y, ys->screen_update->cur_x + i, G(0));
            } else {
                set_cell(ys->screen_update->cur_y, ys->screen_update->cur_x + i, G(0));
            }
        }

        ys->screen_update->cur_x += width;

        s = s + len;
    }
}

void yed_screen_print(const char *s)               { if (!s) { return; } yed_screen_print_n(s, strlen(s));      }
void yed_screen_print_n(const char *s, int n)      { if (!s) { return; } screen_print_n(s, n, 0);               }
void yed_screen_print_over(const char *s)          { if (!s) { return; } yed_screen_print_n_over(s, strlen(s)); }
void yed_screen_print_n_over(const char *s, int n) { if (!s) { return; } screen_print_n(s, strlen(s), 1);       }
