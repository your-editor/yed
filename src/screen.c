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

void yed_set_attr(yed_attrs attr) {
    ys->screen_update->cur_attrs = attr;
}

void yed_reset_attr(void) {
    ys->screen_update->cur_attrs = ZERO_ATTR;
}

static void set_cell(int row, int col, yed_glyph g) {
    yed_screen_cell *cell;

    cell = ys->screen_update->cells + ((row - 1) * ys->term_cols) + (col - 1);

    cell->attrs = ys->screen_update->cur_attrs;
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

    ucell = ys->screen_update->cells;
    rcell = ys->screen_render->cells;

    for (row = 1; row <= ys->term_rows; row += 1) {
        for (col = 1; col <= ys->term_cols; col += 1) {
            ucell->dirty = (   ucell->glyph.data != rcell->glyph.data
                            || !yed_attrs_eq(ucell->attrs, rcell->attrs));

            ucell += 1;
            rcell += 1;
        }
    }

    XOR_SWAP_PTR(ys->screen_update, ys->screen_render);

    memcpy(ys->screen_update->cells,
           ys->screen_render->cells,
           ys->term_rows * ys->term_cols * sizeof(yed_screen_cell));
}

void yed_render_screen(void) {
    array_t          output_buffer;
    yed_screen_cell *cell;
    int              row;
    int              col;
    char             buff[512];
    int              cursor_x;
    int              cursor_y;

#define WR(s, n) array_push_n(output_buffer, s, n)

LOG_FN_ENTER();

    output_buffer = array_make(char);

    WR(TERM_CURSOR_HIDE, strlen(TERM_CURSOR_HIDE));

    cell = ys->screen_render->cells;

    ys->screen_render->cur_x = ys->screen_render->cur_y = 1;
    snprintf(buff, sizeof(buff), "%s1%s1%s",
        TERM_CURSOR_MOVE_BEG,
        TERM_CURSOR_MOVE_SEP,
        TERM_CURSOR_MOVE_END);
    WR(buff, strlen(buff));

    ys->screen_render->cur_attrs = ZERO_ATTR;
    WR(TERM_RESET, strlen(TERM_RESET));

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
                    ys->screen_render->cur_attrs = cell->attrs;
                    yed_get_attr_str(cell->attrs, buff);
                    WR(buff, strlen(buff));
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

    write(1, array_data(output_buffer), array_len(output_buffer));

    array_free(output_buffer);

LOG_EXIT();

#undef WR
}

void yed_screen_print(const char *s) { yed_screen_print_n(s, strlen(s)); }

void yed_screen_print_n(const char *s, int n) {
    const char *end;
    yed_glyph  *g;
    int         len;
    int         width;
    yed_glyph   new_g;
    int         i;

    end = s + n;

    while (s < end) {
        g     = (yed_glyph*)(void*)s;
        len   = yed_get_glyph_len(*g);
        width = yed_get_glyph_width(*g);

        new_g = G(0);
        memcpy(&new_g, g, len);

        set_cell(ys->screen_update->cur_y, ys->screen_update->cur_x, new_g);

        for (i = 1; i < width; i += 1) {
            set_cell(ys->screen_update->cur_y, ys->screen_update->cur_x + i, G(0));
        }

        ys->screen_update->cur_x += width;

        s = s + len;
    }
}
