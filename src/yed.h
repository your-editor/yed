#ifndef __YED_H__
#define __YED_H__

struct yed_state_t;

#define YED_NORMAL      (0x1)
#define YED_QUIT        (0x2)
#define YED_RELOAD      (0x3)
#define YED_RELOAD_CORE (0x4)

typedef struct yed_lib {
    void                *handle;
    struct yed_state_t* (*_init)(struct yed_lib *, int, char**);
    void                (*_fini)(struct yed_state_t*);
    int                 (*_pump)(void);
    struct yed_state_t* (*_get_state)(void);
    void                (*_set_state)(struct yed_state_t*);
} yed_lib_t;


struct yed_state_t* yed_init(yed_lib_t *, int argc, char **argv);
void                yed_fini(struct yed_state_t *state);
int                 yed_pump(void);
void                yed_set_state(struct yed_state_t *state);
struct yed_state_t* yed_get_state(void);

#endif
