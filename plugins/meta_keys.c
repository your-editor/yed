#include <yed/plugin.h>

int yed_plugin_boot(yed_plugin *self) {
    int  key, keys[2], virt_key;
    char vkey_buff[32];
    char var_buff[64];

    keys[0] = ESC;

    for (key = 1; key < REAL_KEY_MAX; key += 1) {
        /*
         * These are reserved so that built in ESC
         * sequences (like arrow keys) still work.
         */
        if (key == '[' || key == 'O')    { continue; }


        keys[1]  = key;
        virt_key = yed_plugin_add_key_sequence(self, 2, keys);
        sprintf(vkey_buff, "%d", virt_key);
        sprintf(var_buff, "meta-keys-M%d", key);
        yed_set_var(var_buff, vkey_buff);
    }

    return 0;
}
