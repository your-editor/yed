#define FT_UNKNOWN       (-1)
#define FT_ERR_TAKEN     (-2)
#define FT_ERR_NOT_FOUND (-3)

void   yed_init_ft(void);
int    yed_make_ft(char *name);
void   yed_delete_ft(char *name);
int    yed_get_ft(char *name);
char * yed_get_ft_name(int ft);
