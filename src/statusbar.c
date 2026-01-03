#include "statusbar.h"
#include <stdio.h>

void statusbar_render(Console *con, TailPane *panes, int pane_count, int active_pane) {
    if (!con || !panes || pane_count <= 0) {
        return;
    }

    int status_row = con->height - 1;

    // Fill status bar background
    console_fill_row(con, status_row, ' ', COLOR_STATUS);

    // Get active pane info
    TailPane *active = &panes[active_pane];
    size_t line_count = linebuf_count(&active->buffer);
    size_t view_line = active->view_line;

    // Build status text
    char status[256];
    if (active->following) {
        snprintf(status, sizeof(status),
            " Pane %d/%d | LIVE (%zu lines) | Tab:next  Arrows:scroll  End:follow  Q:quit",
            active_pane + 1, pane_count, line_count);
    } else {
        // Calculate visible range
        size_t view_end = view_line + active->content_height;
        if (view_end > line_count) {
            view_end = line_count;
        }

        snprintf(status, sizeof(status),
            " Pane %d/%d | SCROLL %zu-%zu/%zu | Tab:next  Arrows:scroll  End:follow  Q:quit",
            active_pane + 1, pane_count,
            view_line + 1, view_end, line_count);
    }

    console_write_at(con, status_row, 0, status, COLOR_STATUS);
}
