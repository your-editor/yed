#ifndef __UTIL_H__
#define __UTIL_H__

char * yed_word_under_cursor(void);

char * get_path_ext(char *path);     /* Don't free result. */
char * path_without_ext(char *path); /* DO free result. */

char *exe_path(char *prg); /* Free the result. */

int perc_subst(char *pattern, char *subst, char *buff, int buff_len);

#endif
