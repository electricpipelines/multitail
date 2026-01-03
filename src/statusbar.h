#ifndef STATUSBAR_H
#define STATUSBAR_H

#include "console.h"
#include "pane.h"

// Render the status bar at the bottom of the console
void statusbar_render(Console *con, TailPane *panes, int pane_count, int active_pane);

#endif // STATUSBAR_H
