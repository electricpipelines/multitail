#include "input.h"

InputAction input_poll(Console *con) {
    if (!con) {
        return INPUT_NONE;
    }

    // Check if there are input events available
    DWORD num_events;
    if (!GetNumberOfConsoleInputEvents(con->in_handle, &num_events) || num_events == 0) {
        return INPUT_NONE;
    }

    INPUT_RECORD record;
    DWORD events_read;

    while (num_events > 0) {
        if (!ReadConsoleInputA(con->in_handle, &record, 1, &events_read) || events_read == 0) {
            break;
        }
        num_events--;

        if (record.EventType == WINDOW_BUFFER_SIZE_EVENT) {
            return INPUT_RESIZE;
        }

        if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
            KEY_EVENT_RECORD *key = &record.Event.KeyEvent;
            WORD vk = key->wVirtualKeyCode;
            DWORD ctrl_state = key->dwControlKeyState;

            // Check for Ctrl+C
            if (vk == 'C' && (ctrl_state & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))) {
                return INPUT_QUIT;
            }

            // Check for 'q' or 'Q'
            if (vk == 'Q') {
                return INPUT_QUIT;
            }

            // Tab navigation
            if (vk == VK_TAB) {
                if (ctrl_state & SHIFT_PRESSED) {
                    return INPUT_TAB_PREV;
                }
                return INPUT_TAB_NEXT;
            }

            // Arrow keys
            if (vk == VK_UP) {
                return INPUT_SCROLL_UP;
            }
            if (vk == VK_DOWN) {
                return INPUT_SCROLL_DOWN;
            }

            // Page up/down
            if (vk == VK_PRIOR) {  // Page Up
                return INPUT_PAGE_UP;
            }
            if (vk == VK_NEXT) {   // Page Down
                return INPUT_PAGE_DOWN;
            }

            // Home/End
            if (vk == VK_HOME) {
                return INPUT_HOME;
            }
            if (vk == VK_END) {
                return INPUT_END;
            }
        }
    }

    return INPUT_NONE;
}
