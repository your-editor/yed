#ifndef __FIND_H__
#define __FIND_H__

void yed_init_search(void);
void yed_search_line_handler(yed_event *event);
int  yed_find_next(int row, int col, int *row_out, int *col_out);
int  yed_find_prev(int row, int col, int *row_out, int *col_out);

#endif
