#ifndef __PACK_STYLES_H__
#define __PACK_STYLES_H__

#ifdef STYLE_PACK
#define PACKABLE_STYLE(name) \
    int _packable_style_##name(yed_plugin *self)
#else
#define PACKABLE_STYLE(name) \
    int yed_plugin_boot(yed_plugin *self)
#endif

#define PACK_STYLE(plug, name) \
    _packable_style_##name((plug))

#endif
