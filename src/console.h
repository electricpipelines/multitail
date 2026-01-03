#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>
#include <windows.h>

// Color attributes
#define COLOR_DEFAULT       (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define COLOR_HEADER        (BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define COLOR_HEADER_ACTIVE (BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define COLOR_STATUS        (BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define COLOR_SEPARATOR     (FOREGROUND_BLUE | FOREGROUND_INTENSITY)

typedef struct {
    HANDLE out_handle;
    HANDLE in_handle;
    DWORD original_out_mode;
    DWORD original_in_mode;
    WORD original_attributes;
    int width;
    int height;
} Console;

// Initialize console, hide cursor, set modes
bool console_init(Console *con);

// Restore original console state
void console_cleanup(Console *con);

// Update cached console size. Returns true if size changed.
bool console_update_size(Console *con);

// Write text at specific position with given attributes
void console_write_at(Console *con, int row, int col, const char *text, WORD attr);

// Write text at position, truncating/padding to fit width
void console_write_fixed(Console *con, int row, int col, const char *text, int width, WORD attr);

// Fill a row with a character
void console_fill_row(Console *con, int row, char ch, WORD attr);

// Clear entire screen
void console_clear(Console *con);

#endif // CONSOLE_H
