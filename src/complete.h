#ifndef __COMPLETE_H__
#define __COMPLETE_H__

#include "internal.h"

#define COMPL_CMD   (1)
#define COMPL_BUFF  (2)
#define COMPL_PATH  (3)

int yed_get_completion(int type, char *in, char ***out, int *common_prefix_len);

#endif
