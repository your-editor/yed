/*
 * Welcome to yed!
 * This file is meant to guide you through the process of using yed and
 * creating/managing your own yed configuration.
 *
 * If you need to quit, press ctrl-p, type "quit", and hit ENTER.
 *
 * Right now, you are using yed without any configuration or plugins.
 * To navigate and edit this file in yed's basic state, use the
 * following keybindings:
 *     Move the cursor:                               arrow keys
 *     Insert a character:                            type the key
 *     Delete the previous character:                 hit the backspace key
 *     Delete the next character:                     hit the delete key
 *     Delete the current line:                       ctrl-d
 *     Select/deselect text:                          ctrl-s
 *     Yank selected text:                            ctrl-c
 *     Paste from yank buffer:                        ctrl-v
 *     Undo:                                          ctrl-u
 *     Redo:                                          ctrl-r
 *     Search for and jump to a string in the buffer: ctrl-f
 *     Write the buffer to disk:                      ctrl-w
 *     Run a command:                                 ctrl-p
 *     Quit:                                          run the command 'quit'
 */

/*
 * Let's get familiar with running a command in yed.
 * To access the command prompt, use the default keybinding ctrl-p.
 * A colon (:) should appear at the bottom of the screen (the command line).
 *
 * Try typing "style" into the command prompt and hit ENTER.
 *
 * "style" is a command that reports the active style when given no
 * arguments, or activates a style when givin the name of one as an
 * argument. You're likely using the style "default".
 *
 * An important command that we're going to use a lot soon is "reload".
 * "reload" allows any configuration from plugins to be reloaded on the fly
 * and for the changes to be made available immediately.
 * There is no need to restart yed when changing your configuration.
 *
 * Run the command "reload".
 * Nothing has changed.
 * Run the command "plugins-list" -- the only output should be "init".
 * That's this file!
 * At this point, this is the only source of configuration for yed.
 */

/* This file needs to interact with the yed core. */
#include <yed/plugin.h>

/*
 * Below is a simple implementation of a command that we're going to use to
 * compile this file when you make changes to it.
 * It is short and easy to follow, but you don't need to worry about its
 * implementation.
 */
void compile_this_file(int n_args, char **args);

/* This is the entry point for this file when yed loads it. */
int yed_plugin_boot(yed_plugin *self) {
    /* This makes the compile_this_file function available as a command. */
    yed_plugin_set_command(self, "compile-this-file", compile_this_file);

    /*
     * yed allows for configuration as deep as fundamental editing style.
     * So the first thing we'll do is load an editor emulation plugin.
     * Or you can leave yed how it is if you prefer.
     *
     * NOTE: YEXE() is just a macro that allows you to call into yed
     * commands programmatically.
     */

    /* If you prefer a vim-like editing style, uncomment the next line. */
    /* YEXE("plugin-load", "vimish"); */

    /*
     * If you prefer an emacs-like editing style, uncomment following
     * lines.
     * NOTE: the yedmacs plugin is VERY incomplete. In fact, it pretty
     * much doesn't work at all! (Help wanted.)
     */
    /* YEXE("plugin-load", "meta_keys"); */
    /* YEXE("plugin-load", "yedmacs"); */

    /*
     * If you've made changes to the file, write the changes
     * using the command "write-buffer" (default bound to
     * ctrl-w).
     * Then you can recompile by running "compile-this-file".
     * Finally, to make the changes take effect, run "reload".
     * This is the workflow that we'll be using from now on to
     * change this configuration.
     */

    /*
     * NOTE: If at any time you wish to get rid of a plugin that
     * you've loaded, just run "plugin-unload <name of plugin>".
     */




    /*
     * Now, perhaps it would be easier to edit this configuration
     * (written in C in case you haven't noticed) if it had some
     * syntax highlighting.
     * For this, we can load plugin that implements a syntax highlighter for
     * C buffers, enabled by uncommenting the next line, recompiling, and
     * reloading.
     */
    /* YEXE("plugin-load", "lang/c"); */

    /* These other language plugins are available: */
    /* YEXE("plugin-load", "lang/bjou"); */
    /* YEXE("plugin-load", "lang/jgraph"); */
    /* YEXE("plugin-load", "lang/latex"); */
    /* YEXE("plugin-load", "lang/python"); */
    /* YEXE("plugin-load", "lang/sh"); */
    /* YEXE("plugin-load", "lang/yedrc"); */




    /*
     * yed's styling is also implemented in terms of plugins, so we can change
     * that in a similar way.
     * Uncomment the following block to load some style plugins.
     * Note that this won't activate any styles, it will just make
     * them available.
     */

    /*
    YEXE("plugin-load", "styles/blue");
    YEXE("plugin-load", "styles/cadet");
    YEXE("plugin-load", "styles/casey");
    YEXE("plugin-load", "styles/first");
    YEXE("plugin-load", "styles/gruvbox");
    YEXE("plugin-load", "styles/monokai");
    YEXE("plugin-load", "styles/nord");
    YEXE("plugin-load", "styles/papercolor");
    */

    /*
     * If you run the command "styles-list", you should see a list of all the
     * styles provided by those plugins.
     * Now, try activating one.
     * After this comment, add a YEXE() call that runs the command
     * "style <name of style>".
     * After recompiling and reloading, you should see some refreshed colors!
     */



     /*
      * Typically, core and plugin options are configured via yed
      * variables.
      * The commands "set", "unset", and "get" are available to
      * manipulate these variables.
      * As an example, run the command "set cursor-line yes".
      *
      * To make this change persist, add a line below that calls the
      * "set" command with those arguments and it will be run any time
      * this configuration is loaded.
      */





      /*
       * This is a small example of how to configure yed, but the
       * intention is that you are aware of how it works and now have a
       * place to start from.
       *
       * If you copy the files generated from this program (the paths will be
       * reported after you close this program) to ~/.yed, this will be
       * loaded as your default configuration the next time you run yed.
       *
       * Then, you can always go and edit this configuration from there.
       * Or, put it in source control.
       * Or, do whatever you want -- it's your editor.
       */

    return 0;
}

void compile_this_file(int n_args, char **args) {
    char *path;
    char  cmd_buff[256];

    if (!ys->active_frame || !ys->active_frame->buffer) {
        yed_cerr("No file to compile!");
        return;
    }

    path = ys->active_frame->buffer->path;

    if (!path) {
        yed_cerr("current buffer does not have a path");
        return;
    }

    sprintf(cmd_buff,
            "gcc '%s' -fPIC -shared -lyed -o init.so && echo success",
            path);

    YEXE("sh", cmd_buff);
}
