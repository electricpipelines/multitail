#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#include "console.h"
#include "pane.h"
#include "input.h"
#include "statusbar.h"

#define POLL_INTERVAL_MS 50

typedef struct {
    Console console;
    TailPane panes[MAX_PANES];
    int pane_count;
    int active_pane;
    bool running;
} MultiTail;

static void print_usage(const char *prog) {
    fprintf(stderr, "Usage: %s <file1> [file2] ... [file8]\n", prog);
    fprintf(stderr, "Tail multiple files simultaneously.\n\n");
    fprintf(stderr, "Controls:\n");
    fprintf(stderr, "  Tab        - Switch to next pane\n");
    fprintf(stderr, "  Shift+Tab  - Switch to previous pane\n");
    fprintf(stderr, "  Up/Down    - Scroll in active pane\n");
    fprintf(stderr, "  PgUp/PgDn  - Scroll by page\n");
    fprintf(stderr, "  Home       - Jump to start of buffer\n");
    fprintf(stderr, "  End        - Resume live following\n");
    fprintf(stderr, "  Q / Ctrl+C - Quit\n");
}

static void calculate_pane_regions(MultiTail *app) {
    // Reserve 1 row for status bar
    int available = app->console.height - 1;
    int pane_height = available / app->pane_count;
    int remainder = available % app->pane_count;

    int top = 0;
    for (int i = 0; i < app->pane_count; i++) {
        int height = pane_height;
        // Distribute remainder rows to first panes
        if (i < remainder) {
            height++;
        }
        pane_set_region(&app->panes[i], top, height);
        top += height;
    }
}

static void handle_input(MultiTail *app, InputAction action) {
    TailPane *active = &app->panes[app->active_pane];

    switch (action) {
        case INPUT_QUIT:
            app->running = false;
            break;

        case INPUT_TAB_NEXT:
            app->active_pane = (app->active_pane + 1) % app->pane_count;
            break;

        case INPUT_TAB_PREV:
            app->active_pane = (app->active_pane - 1 + app->pane_count) % app->pane_count;
            break;

        case INPUT_SCROLL_UP:
            pane_scroll_up(active);
            break;

        case INPUT_SCROLL_DOWN:
            pane_scroll_down(active);
            break;

        case INPUT_PAGE_UP:
            pane_page_up(active);
            break;

        case INPUT_PAGE_DOWN:
            pane_page_down(active);
            break;

        case INPUT_HOME:
            pane_scroll_home(active);
            break;

        case INPUT_END:
            pane_scroll_end(active);
            break;

        case INPUT_RESIZE:
            console_update_size(&app->console);
            calculate_pane_regions(app);
            console_clear(&app->console);
            break;

        case INPUT_NONE:
        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (argc - 1 > MAX_PANES) {
        fprintf(stderr, "Error: Maximum of %d files allowed.\n", MAX_PANES);
        return 1;
    }

    MultiTail app = {0};
    app.running = true;
    app.active_pane = 0;
    app.pane_count = 0;

    // Initialize console first
    if (!console_init(&app.console)) {
        fprintf(stderr, "Error: Failed to initialize console.\n");
        return 1;
    }

    // Initialize panes for each file
    for (int i = 1; i < argc; i++) {
        if (!pane_init(&app.panes[app.pane_count], argv[i])) {
            fprintf(stderr, "Error: Failed to open file: %s\n", argv[i]);
            // Cleanup already initialized panes
            for (int j = 0; j < app.pane_count; j++) {
                pane_destroy(&app.panes[j]);
            }
            console_cleanup(&app.console);
            return 1;
        }
        app.pane_count++;
    }

    // Calculate initial pane regions
    calculate_pane_regions(&app);

    int prev_active = -1;  // Track active pane changes

    // Main loop
    while (app.running) {
        // Handle input
        InputAction action = input_poll(&app.console);
        handle_input(&app, action);

        if (!app.running) {
            break;
        }

        // Update all panes (check for new file content)
        for (int i = 0; i < app.pane_count; i++) {
            pane_update(&app.panes[i]);
        }

        // Check if active pane changed
        bool active_changed = (prev_active != app.active_pane);
        if (active_changed) {
            // Mark both old and new active panes as dirty for header update
            if (prev_active >= 0 && prev_active < app.pane_count) {
                app.panes[prev_active].dirty = true;
            }
            app.panes[app.active_pane].dirty = true;
            prev_active = app.active_pane;
        }

        // Check if any pane needs redraw
        bool needs_redraw = false;
        for (int i = 0; i < app.pane_count; i++) {
            if (app.panes[i].dirty) {
                needs_redraw = true;
                break;
            }
        }

        // Only render if something changed
        if (needs_redraw) {
            for (int i = 0; i < app.pane_count; i++) {
                if (app.panes[i].dirty) {
                    pane_render(&app.panes[i], &app.console, i == app.active_pane);
                }
            }
            statusbar_render(&app.console, app.panes, app.pane_count, app.active_pane);
        }

        // Small sleep to avoid busy-waiting
        Sleep(POLL_INTERVAL_MS);
    }

    // Cleanup
    for (int i = 0; i < app.pane_count; i++) {
        pane_destroy(&app.panes[i]);
    }
    console_cleanup(&app.console);

    return 0;
}
