#include <yed/plugin.h>

int meta_key(int key);
void bind_keys(yed_plugin *self);

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    bind_keys(self);

    return 0;
}

void bind_keys(yed_plugin *self) {
    int key;

    /* Clear any existing bindings. */
    for (key = 1; key < REAL_KEY_MAX; key += 1) {
        yed_unbind_key(key);
    }

#define BIND(key, cmd) yed_plugin_bind_key(self, (key), (cmd), 0, NULL)

    BIND(CTRL_F,        "cursor-right");
    BIND(CTRL_B,        "cursor-left");
    BIND(CTRL_N,        "cursor-down");
    BIND(CTRL_P,        "cursor-up");
    BIND(CTRL_A,        "cursor-line-begin");
    BIND(CTRL_E,        "cursor-line-end");
    BIND(meta_key('f'), "cursor-next-word");
    BIND(meta_key('b'), "cursor-prev-word");
    BIND(meta_key('a'), "cursor-next-paragraph");
    BIND(meta_key('e'), "cursor-prev-paragraph");
    BIND(CTRL_V,        "cursor-page-down");
    BIND(meta_key('v'), "cursor-page-up");
    BIND(meta_key('<'), "cursor-buffer-begin");
    BIND(meta_key('>'), "cursor-buffer-end");

    BIND(CTRL_S,        "find-in-buffer");
    BIND(CTRL_R,        "find-prev-in-buffer");

    BIND(CTRL_K,        "delete-line");

    BIND(CTRL_Y,        "paste-yank-buffer");

    BIND(CTRL_FS,       "undo");

    BIND(meta_key('x'), "command-prompt");
}

int meta_key(int key) {
    char  var_buff[64];
    char *key_str;
    int   vkey;

    sprintf(var_buff, "meta-keys-M%d", key);

    key_str = yed_get_var(var_buff);

    if (!key_str) {
        return KEY_NULL;
    }

    sscanf(key_str, "%d", &vkey);

    return vkey;
}
