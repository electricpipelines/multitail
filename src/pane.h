#ifndef PANE_H
#define PANE_H

#include <stdbool.h>
#include <windows.h>
#include "linebuf.h"
#include "console.h"

#define MAX_PANES 8
#define READ_BUFFER_SIZE 65536

typedef struct {
    char filepath[MAX_PATH];   // File being tailed
    HANDLE file_handle;        // File handle for reading
    LONGLONG read_pos;         // Current read position in file

    LineBuffer buffer;         // Scrollback buffer
    size_t view_line;          // Top line of current view (logical index)
    bool following;            // True = auto-scroll to new content

    char *partial_line;        // Incomplete line from last read
    size_t partial_len;        // Length of partial line

    // Display region
    int top_row;               // Console row where pane starts
    int height;                // Pane height in rows (including header)
    int content_height;        // Height available for content (height - 1 for header)

    bool dirty;                // True if pane needs redraw
} TailPane;

// Initialize a pane for the given file path
bool pane_init(TailPane *pane, const char *filepath);

// Free pane resources
void pane_destroy(TailPane *pane);

// Check file for new content and read it into buffer
void pane_update(TailPane *pane);

// Render the pane to the console
void pane_render(TailPane *pane, Console *con, bool is_active);

// Scroll up by one line (returns true if scrolled)
bool pane_scroll_up(TailPane *pane);

// Scroll down by one line (returns true if scrolled)
bool pane_scroll_down(TailPane *pane);

// Scroll up by a page
void pane_page_up(TailPane *pane);

// Scroll down by a page
void pane_page_down(TailPane *pane);

// Jump to start of buffer
void pane_scroll_home(TailPane *pane);

// Resume following mode (jump to end)
void pane_scroll_end(TailPane *pane);

// Set pane display region
void pane_set_region(TailPane *pane, int top_row, int height);

#endif // PANE_H
