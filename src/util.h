#ifndef __UTIL_H__
#define __UTIL_H__

char * yed_word_under_cursor(void);

char * abs_path(char *path, char *buff);
char * relative_path_if_subtree(char *path, char *buff);
char * homeify_path(char *path, char *buff);
char * get_path_ext(char *path);      /* Don't free result. */
char * get_path_basename(char *path); /* Don't free result. */
char * path_without_ext(char *path);  /* DO free result. */

char *exe_path(char *prg); /* Free the result. */

int perc_subst(char *pattern, char *subst, char *buff, int buff_len);

void expand_path(char *path, char *buff);

int file_exists_in_path(char *path, char *name);
int file_exists_in_PATH(char *name);

array_t sh_split(char *s);

void free_string_array(array_t array);

char *last_strstr(const char *haystack, const char *needle);
char *last_strnstr(const char *haystack, const char *needle, size_t len);

#ifdef NEED_STRNSTR
char *strnstr(const char *haystack, const char *needle, size_t len);
#endif

int rect_intersect(int top_a, int bottom_a, int left_a, int right_a,
                   int top_b, int bottom_b, int left_b, int right_b);

#endif
