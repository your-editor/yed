#ifndef __UTIL_H__
#define __UTIL_H__

char * yed_word_under_cursor(void);

char * get_path_ext(char *path);     /* Don't free result. */
char * path_without_ext(char *path); /* DO free result. */

#endif
