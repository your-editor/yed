#ifndef __VAR_H__
#define __VAR_H__

#include "internal.h"

typedef char *yed_var_name_t;
typedef char *yed_var_val_t;

void yed_init_vars(void);

void yed_set_default_vars(void);

void yed_set_var(char *var, char *val);
char *yed_get_var(char *var);
void yed_unset_var(char *var);

#endif
