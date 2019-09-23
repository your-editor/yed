#ifndef __KEY_H__
#define __KEY_H__

#include "internal.h"

#undef CTRL
#define CTRL(k)  ((k) & 0x1f)
#define NCTRL(k) ((k) | 0x40)

#define KEY_UP      (301)
#define KEY_DOWN    (302)
#define KEY_LEFT    (303)
#define KEY_RIGHT   (304)
#define IS_ARROW(k) ((k) >= KEY_UP && (k) <= KEY_RIGHT)

static int yed_read_key(void);

#endif
