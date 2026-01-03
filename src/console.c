#include "console.h"
#include <string.h>

bool console_init(Console *con) {
    if (!con) {
        return false;
    }

    // Get handles
    con->out_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    con->in_handle = GetStdHandle(STD_INPUT_HANDLE);

    if (con->out_handle == INVALID_HANDLE_VALUE ||
        con->in_handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Save original modes
    GetConsoleMode(con->out_handle, &con->original_out_mode);
    GetConsoleMode(con->in_handle, &con->original_in_mode);

    // Save original attributes
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(con->out_handle, &csbi)) {
        con->original_attributes = csbi.wAttributes;
    } else {
        con->original_attributes = COLOR_DEFAULT;
    }

    // Set input mode: disable line input, enable window/mouse events
    DWORD in_mode = ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT;
    SetConsoleMode(con->in_handle, in_mode);

    // Hide cursor
    CONSOLE_CURSOR_INFO cursor_info;
    cursor_info.dwSize = 100;
    cursor_info.bVisible = FALSE;
    SetConsoleCursorInfo(con->out_handle, &cursor_info);

    // Get initial size
    console_update_size(con);

    // Clear screen
    console_clear(con);

    return true;
}

void console_cleanup(Console *con) {
    if (!con) {
        return;
    }

    // Restore modes
    SetConsoleMode(con->out_handle, con->original_out_mode);
    SetConsoleMode(con->in_handle, con->original_in_mode);

    // Show cursor
    CONSOLE_CURSOR_INFO cursor_info;
    cursor_info.dwSize = 100;
    cursor_info.bVisible = TRUE;
    SetConsoleCursorInfo(con->out_handle, &cursor_info);

    // Restore attributes
    SetConsoleTextAttribute(con->out_handle, con->original_attributes);

    // Clear and reset cursor
    console_clear(con);
    COORD pos = {0, 0};
    SetConsoleCursorPosition(con->out_handle, pos);
}

bool console_update_size(Console *con) {
    if (!con) {
        return false;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(con->out_handle, &csbi)) {
        return false;
    }

    int new_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int new_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    bool changed = (new_width != con->width || new_height != con->height);
    con->width = new_width;
    con->height = new_height;

    return changed;
}

void console_write_at(Console *con, int row, int col, const char *text, WORD attr) {
    if (!con || !text || row < 0 || col < 0) {
        return;
    }

    COORD pos;
    pos.X = (SHORT)col;
    pos.Y = (SHORT)row;

    SetConsoleCursorPosition(con->out_handle, pos);
    SetConsoleTextAttribute(con->out_handle, attr);

    DWORD written;
    WriteConsoleA(con->out_handle, text, (DWORD)strlen(text), &written, NULL);
}

void console_write_fixed(Console *con, int row, int col, const char *text, int width, WORD attr) {
    if (!con || row < 0 || col < 0 || width <= 0) {
        return;
    }

    // Create a buffer padded with spaces
    char *buf = (char *)malloc(width + 1);
    if (!buf) {
        return;
    }

    memset(buf, ' ', width);
    buf[width] = '\0';

    // Copy text, truncating if necessary
    if (text) {
        size_t len = strlen(text);
        if (len > (size_t)width) {
            len = width;
        }
        memcpy(buf, text, len);
    }

    console_write_at(con, row, col, buf, attr);
    free(buf);
}

void console_fill_row(Console *con, int row, char ch, WORD attr) {
    if (!con || row < 0) {
        return;
    }

    COORD pos;
    pos.X = 0;
    pos.Y = (SHORT)row;

    DWORD written;
    FillConsoleOutputAttribute(con->out_handle, attr, con->width, pos, &written);
    FillConsoleOutputCharacterA(con->out_handle, ch, con->width, pos, &written);
}

void console_clear(Console *con) {
    if (!con) {
        return;
    }

    COORD origin = {0, 0};
    DWORD size = con->width * con->height;
    DWORD written;

    FillConsoleOutputCharacterA(con->out_handle, ' ', size, origin, &written);
    FillConsoleOutputAttribute(con->out_handle, con->original_attributes, size, origin, &written);
}
