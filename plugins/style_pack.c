#define STYLE_PACK
#include <yed/plugin.h>

#include "styles/lab.c"
#include "styles/book.c"
#include "styles/blue.c"
#include "styles/first.c"
#include "styles/elise.c"
#include "styles/nord.c"
#include "styles/monokai.c"
#include "styles/gruvbox.c"
#include "styles/skyfall.c"
#include "styles/papercolor.c"
#include "styles/casey.c"
#include "styles/cadet.c"
#include "styles/moss.c"
#include "styles/hat.c"
#include "styles/dracula.c"
#include "styles/solarized.c"
#include "styles/sammy.c"
#include "styles/tempus_future.c"
#include "styles/olive.c"
#include "styles/vt.c"
#include "styles/vt_light.c"
#include "styles/bold.c"
#include "styles/doug.c"
#include "styles/acme.c"
#include "styles/disco.c"
#include "styles/dalton.c"
#include "styles/embark.c"
#include "styles/bullet.c"
#include "styles/mrjantz.c"
#include "styles/elly.c"
#include "styles/river.c"
#include "styles/mordechai.c"
#include "styles/humanoid.c"
#include "styles/forest.c"

int yed_plugin_boot(yed_plugin *self) {
    YED_PLUG_VERSION_CHECK();

    PACK_STYLE(self, lab);
    PACK_STYLE(self, book);
    PACK_STYLE(self, blue);
    PACK_STYLE(self, first);
    PACK_STYLE(self, elise);
    PACK_STYLE(self, nord);
    PACK_STYLE(self, monokai);
    PACK_STYLE(self, gruvbox);
    PACK_STYLE(self, skyfall);
    PACK_STYLE(self, papercolor);
    PACK_STYLE(self, casey);
    PACK_STYLE(self, cadet);
    PACK_STYLE(self, moss);
    PACK_STYLE(self, hat);
    PACK_STYLE(self, dracula);
    PACK_STYLE(self, solarized);
    PACK_STYLE(self, sammy);
    PACK_STYLE(self, tempus_future);
    PACK_STYLE(self, olive);
    PACK_STYLE(self, vt);
    PACK_STYLE(self, vt_light);
    PACK_STYLE(self, bold);
    PACK_STYLE(self, doug);
    PACK_STYLE(self, acme);
    PACK_STYLE(self, disco);
    PACK_STYLE(self, dalton);
    PACK_STYLE(self, embark);
    PACK_STYLE(self, bullet);
    PACK_STYLE(self, mrjantz);
    PACK_STYLE(self, elly);
    PACK_STYLE(self, river);
    PACK_STYLE(self, mordechai);
    PACK_STYLE(self, humanoid);
    PACK_STYLE(self, forest);

    return 0;
}
