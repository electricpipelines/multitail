#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include "console.h"
#include "pane.h"

// Input action types
typedef enum {
    INPUT_NONE,
    INPUT_QUIT,
    INPUT_TAB_NEXT,
    INPUT_TAB_PREV,
    INPUT_SCROLL_UP,
    INPUT_SCROLL_DOWN,
    INPUT_PAGE_UP,
    INPUT_PAGE_DOWN,
    INPUT_HOME,
    INPUT_END,
    INPUT_RESIZE
} InputAction;

// Poll for input (non-blocking). Returns the action type.
InputAction input_poll(Console *con);

#endif // INPUT_H
