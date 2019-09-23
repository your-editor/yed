#ifndef __YED_H__
#define __YED_H__

struct yed_state_t;

#define YED_NORMAL (0x1)
#define YED_QUIT   (0x2)
#define YED_RELOAD (0x3)

struct yed_state_t* yed_init(int argc, char **argv);
void                yed_fini(struct yed_state_t *state);
int                 yed_pump(void);
void                yed_set_state(struct yed_state_t *state);
struct yed_state_t* yed_get_state(void);

#endif
