# Changelog

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
