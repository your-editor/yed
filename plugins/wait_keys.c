#include <yed/plugin.h>

void wait_keys_define(int n_args, char **args);
int  wait_keys_parse(int *keys, char **args);
void wait_keys_take_key(int n_args, char **args);
void wait_keys_bind_start_key(int key);
void wait_keys_unbind_start_key(int key);

static yed_plugin *Self;

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    Self = self;

    yed_plugin_set_command(Self, "wait-keys-define",   wait_keys_define);
    yed_plugin_set_command(Self, "wait-keys-take-key", wait_keys_take_key);

    return 0;
}

void wait_keys_bind_start_key(int key) {
    char key_str[32];
    sprintf(key_str, "%d", key);
    YPBIND(Self, key, "wait-keys-take-key", key_str);
}

void wait_keys_unbind_start_key(int key) {
    yed_unbind_key(key);
}

void wait_keys_define(int n_args, char **args) {
    int  keys[2];
    int  vkey;
    char key_str[32], var_name[128];

    if (n_args != 2) {
        yed_cerr("expected 2 arguments but got %d", n_args);
        return;
    }

    if (!wait_keys_parse(keys, args))    { return; }

    vkey = yed_plugin_acquire_virt_key(Self);

    sprintf(key_str, "%d", vkey);
    sprintf(var_name, "wait-key-%d-%d", keys[0], keys[1]);

    yed_set_var(var_name, key_str);

    wait_keys_bind_start_key(keys[0]);
}

int wait_keys_parse(int *keys, char **args) {
    int   i, key_i;
    char *key_str;
    char  key_c;

    for (i = 0; i < 2; i += 1) {
        key_str = args[i];
        key_c   = key_i = -1;
        if (strlen(key_str) == 1) {
            sscanf(key_str, "%c", &key_c);
            key_i = key_c;
        } else if (sscanf(key_str, "%d", &key_i)) {
            /* The argument is a numerically-described virtual key. */
        } else if (strcmp(key_str, "tab") == 0) {
            key_i = TAB;
        } else if (strcmp(key_str, "enter") == 0) {
            key_i = ENTER;
        } else if (strcmp(key_str, "esc") == 0) {
            key_i = ESC;
        } else if (strcmp(key_str, "spc") == 0) {
            key_i = ' ';
        } else if (strcmp(key_str, "bsp") == 0) {
            key_i = BACKSPACE;
        } else if (sscanf(key_str, "ctrl-%c", &key_c)) {
            if (key_c == '/') {
                key_i = CTRL_FS;
            } else {
                key_i = CTRL_KEY(key_c);
            }
        }

        if (key_i == -1 || !yed_is_key(key_i)) {
            yed_cerr("invalid key '%s'", key_str);
            return 0;
        }

        keys[i] = key_i;
    }

    return 1;
}

void wait_keys_take_key(int n_args, char **args) {
    int start_key, next_key, vkey, rest_start;
    int n_keys, keys[16], n_feed, feed[16], i;
    char var_name[128], *vkey_str;

    if (n_args != 1) {
        yed_cerr("expected 1 argument but got %d", n_args);
        return;
    }

    sscanf(args[0], "%d", &start_key);

    n_feed = 0;
    n_keys = yed_read_keys(keys);

    if (n_keys == 0) {
        n_feed  = 1;
        feed[0] = start_key;
    } else {
        next_key = keys[0];

        sprintf(var_name, "wait-key-%d-%d", start_key, next_key);

        vkey_str = yed_get_var(var_name);

        if (vkey_str) {
            /*
             * It is defined. Translate the first two keys into the virtual key.
             * Feed it and the rest of the keys.
             */
            sscanf(vkey_str, "%d", &vkey);
            n_feed     = 1;
            feed[0]    = vkey;
            rest_start = 1;
        } else {
            /* No match. Feed all keys. */
            n_feed     = 1;
            feed[0]    = start_key;
            rest_start = 0;
        }

        for (i = rest_start; i < n_keys; i += 1) {
            feed[n_feed] = keys[i];
            n_feed += 1;
        }
    }

    wait_keys_unbind_start_key(start_key);
    yed_feed_keys(n_feed, feed);
    wait_keys_bind_start_key(start_key);
}
