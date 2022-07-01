# Changelog

## 1500 - 2022-7-1
### Fixed
    - Some macros contained code that caused compiler warnings or errors when compiled as C++.
    - `yed_buff_get_glyph()` now returns `NULL` if the given position is the end of the line.
### Added
    - `yed_frame_set_name()`: set or clear a name for a frame.
    - `yed_find_frame_by_name()`: lookup a frame based on its name.
    - New commands `frame-name` and `frame-uname`.
    - New in status line: `%F`: the active frame's name (or `-`, if none).
    - New events:
        - `EVENT_FRAME_PRE_ACTIVATE`
        - `EVENT_FRAME_POST_DELETE`
        - `EVENT_FRAME_PRE_MOVE`
        - `EVENT_FRAME_POST_MOVE`
        - `EVENT_FRAME_PRE_RESIZE`
        - `EVENT_FRAME_POST_RESIZE`
        - `EVENT_PRE_DRAW_EVERYTHING`
        - `EVENT_POST_DRAW_EVERYTHING`

## 1403 - 2022-5-4
### Fixed
    - Fixed a bug in `write_welcome()` that caused segfualts when the horizontal screen sized was too small.

## 1402 - 2022-4-13
### Fixed
    - Fixed a bug in `yed_service_reload()` that caused compilation failure for GCC 12.

## 1401 - 2022-4-8
### Fixed
    - Fixed a bug where the screen needed to be fully redraw when resuming from suspend, but wasn't.
    - Fixed a bug where the screen dimensions could be incorrect after resuming from suspend.

## 1400 - 2022-1-28
### Added
    - Variable minimum update rate set with `yed_set_update_hz()`. This is useful for drawing elements that need to update/move faster than the default key timeout.
    - New command `frame-tree-resize` interactively resizes the root tree of the active frame.
    - New key code `MENU_KEY`.
    - `gui.h`: a header that plugins can use to create useful GUI elements.
    - Added two new events, `EVENT_PRE_DIRECT_DRAWS` and `EVENT_POST_DIRECT_DRAWS` that occur before and after the entire set of direct draws is drawn to the screen.

### Changed
    - Rewrote the drawing system from the ground up for high performance and overall smaller output.
    - `frame-resize` can resize frames that are part of a split frame tree.
    - `frame-move` will move the root frame tree of the active frame if it is part of a split.

### Removed
    - `highlight.h` (deprecated in version 1300) has been removed in favor of `syntax.h`.
    - `ys->redraw`, `ys->redraw_cls` were removed because they are no longer necessary.
    - `frame->dirty`, was removed because it is no longer necessary.

### Fixed
    - Fixed a bug where mouse keycodes are sent to interactive comands.
    - Fixed a bug where multi-byte characters were printed incorrectly to the log buffer.
    - Fixed a bug that caused large pastes (from terminal) to be slow.

## 1306 - 2021-12-13
### Fixed
    - fixed a bug that would cause command line contents to overflow into a new terminal line.

## 1305 - 2021-12-13
### Fixed
    - fixed a bug that would cause some cursor movements to crash on binary buffer contents.

## 1304 - 2021-12-10
### Added
    - new command `frame` activates frame based on their number.

## 1303 - 2021-12-8
### Changed
    - the `plugins` directory is now a `subtree`. This makes installing simpler and allows you to just grab a `tar` of a release without needing `git`.

## 1302 - 2021-12-7
### Changed
    - removed use of `codesign` in the install script for Darwin targets. This isn't needed any more and will help us package `yed`.

## 1301 - 2021-12-7
### Fixed
    - Fixed logic in yed_cell_is_in_frame()

## 1300 - 2021-12-6
### Added
- Started keeping a changelog.
- Started tagging versions in git.
- A new syntax highlighting engine in `syntax.h`.
- Status line additions:
    - `%{...}` syntax for expanding a variable that contains an attribute string
    - Padding and justification on `%` expansions: `%` followed by an integer, `x`, will be padded to `x` characters. If `x` is preceded by a `-`, the text will be left justified. A `=` indicates center justification. The default is right justification. This has no effect on expansions that result in attributes.
    - `%T`: 24-hour clock
- Events:
    - `EVENT_STATUS_LINE_PRE_UPDATE` will trigger before the status line is about to be updated.
    - `EVENT_PLUGIN_MESSAGE` is a new general purpose event that plugins can send and other plugins can handle.
- Mouse support added to the core for left, center, and right buttons as well as up and down scroll wheel. Distinguishes between press, release, and drag.
- Useful functions for getting buffer contents as byte strings:
    - `char *yed_get_selection_text(yed_buffer *buffer);`
    - `char *yed_get_line_text(yed_buffer *buffer, int row);`
    - `char *yed_get_buffer_text(yed_buffer *buffer);`
- Style components:
    - `good`: general indication in the positive (e.g. a notification that a build succeeded)
    - `bad`: general indication in the negative (e.g. a notification that a build failed)
    - `popup`: the foreground and background to be used for popup menus
    - `popup-alt`: the foreground and background to be used for other popup menu elements (e.g. the selected item)
    - `code-variable`: for variables in code
    - `code-field`: for fields of a structure or class
    - `code-escape`: for escape sequences in string literals
### Changed
- `highlight.h` is now deprecated and will be removed in version 1400.
- Status line: `%T`, which used to expand to the filetype, now expands to a 24-hour clock. `%F` now expands to the filetype.
### Removed
- Removed the default command `if`.
### Fixed
- A buffer's undo history is now reset when `buffer-reload` gets run (or whenever the contents of the buffer are loaded from a file).
- Plugins are now install-agnostic -- they will use whatever `libyed.so` is being used by the binary. This fixes a crash that would occur when installations would mix on the same system.
