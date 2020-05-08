
#define FT_UNKNOWN (0)
#define FT_TXT     (1)
#define FT_C       (2)
#define FT_CXX     (3)
#define FT_SH      (4)
#define FT_LATEX   (5)
#define FT_BJOU    (6)
#define FT_PYTHON  (7)
#define FT_YEDRC   (8)
#define FT_JGRAPH  (9)
#define NUM_FT     (10)

typedef struct {
    int    ft;
    struct stat stat_at_open;
} yed_file;

int yed_get_ft(char *path);
