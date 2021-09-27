#ifndef __ATTRS_H__
#define __ATTRS_H__

#define ATTR_NORMAL      (0x0)
#define ATTR_INVERSE     (0x1)
#define ATTR_BOLD        (0x2)
#define ATTR_UNDERLINE   (0x4)
#define ATTR_16_LIGHT_FG (0x8)
#define ATTR_16_LIGHT_BG (0x10)
#define ATTR_16          (0x20)
#define ATTR_256         (0x40)
#define ATTR_RGB         (0x80)

#define ATTR_16_BLACK      (30)
#define ATTR_16_RED        (31)
#define ATTR_16_GREEN      (32)
#define ATTR_16_YELLOW     (33)
#define ATTR_16_BLUE       (34)
#define ATTR_16_MAGENTA    (35)
#define ATTR_16_CYAN       (36)
#define ATTR_16_BROWN      (33)
#define ATTR_16_GREY       (37)

#define RGB_32(r, g, b) (((r) << 16) | ((g) << 8) | (b))
#define RGB_32_r(rgb)   ((rgb) >> 16)
#define RGB_32_g(rgb)   (((rgb) >> 8) & 0xFF)
#define RGB_32_b(rgb)   ((rgb) & 0xFF)
#define RGB_32_hex(x)   (0x##x)

int rgb_to_256(unsigned rgb);

typedef struct {
    uint32_t flags;
    uint32_t fg;
    uint32_t bg;
} yed_attrs;

#define ZERO_ATTR    ((yed_attrs){ 0, 0, 0 })

void yed_get_attr_str(yed_attrs attr, char *buff_p);
void yed_set_attr(yed_attrs attr);
int  yed_attrs_eq(yed_attrs attr1, yed_attrs attr2);
void yed_combine_attrs(yed_attrs *dst, yed_attrs *src);
yed_attrs yed_parse_attrs(char *string);

#endif
