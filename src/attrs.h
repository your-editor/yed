#ifndef __ATTRS_H__
#define __ATTRS_H__

#define ATTR_KIND_NONE         (0x0)
#define ATTR_KIND_16           (0x1)
#define ATTR_KIND_256          (0x2)
#define ATTR_KIND_RGB          (0x3)
#define ATTR_FG_KIND_MASK      (0x3)
#define ATTR_BG_KIND_MASK      (0xc)
#define ATTR_FG_KIND_SHIFT     (0)
#define ATTR_BG_KIND_SHIFT     (2)
#define ATTR_FG_KIND(a)        (((a) & ATTR_FG_KIND_MASK) >> ATTR_FG_KIND_SHIFT)
#define ATTR_BG_KIND(a)        (((a) & ATTR_BG_KIND_MASK) >> ATTR_BG_KIND_SHIFT)
#define ATTR_FG_KIND_BITS(k)   (((k) & 0x3) << ATTR_FG_KIND_SHIFT)
#define ATTR_BG_KIND_BITS(k)   (((k) & 0x3) << ATTR_BG_KIND_SHIFT)

#define ATTR_SET_FG_KIND(a, k)        \
do {                                  \
    (a) &= ~ATTR_FG_KIND_MASK;        \
    (a) |= (k) << ATTR_FG_KIND_SHIFT; \
} while (0)

#define ATTR_SET_BG_KIND(a, k)        \
do {                                  \
    (a) &= ~ATTR_BG_KIND_MASK;        \
    (a) |= (k) << ATTR_BG_KIND_SHIFT; \
} while (0)

#define ATTR_NORMAL      (0x0  << 4)
#define ATTR_INVERSE     (0x1  << 4)
#define ATTR_BOLD        (0x2  << 4)
#define ATTR_UNDERLINE   (0x4  << 4)
#define ATTR_16_LIGHT_FG (0x8  << 4)
#define ATTR_16_LIGHT_BG (0x10 << 4)

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

#define ZERO_ATTR ((yed_attrs){ 0, 0, 0 })

#define ATTRS_EQ(_a, _b)          \
    ( ((_a).fg    == (_b).fg)     \
    & ((_a).bg    == (_b).bg)     \
    & ((_a).flags == (_b).flags))

void yed_get_attr_str(yed_attrs attr, char *buff_p);
int  yed_attrs_eq(yed_attrs attr1, yed_attrs attr2);
yed_attrs yed_parse_attrs(const char *string);

#endif
