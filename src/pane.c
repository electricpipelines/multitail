#include "pane.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void process_read_data(TailPane *pane, const char *data, DWORD len);

bool pane_init(TailPane *pane, const char *filepath) {
    if (!pane || !filepath) {
        return false;
    }

    memset(pane, 0, sizeof(TailPane));
    strncpy(pane->filepath, filepath, MAX_PATH - 1);
    pane->filepath[MAX_PATH - 1] = '\0';

    // Initialize line buffer
    if (!linebuf_init(&pane->buffer, LINEBUF_DEFAULT_CAPACITY)) {
        return false;
    }

    // Open file for reading with share permissions
    pane->file_handle = CreateFileA(
        filepath,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (pane->file_handle == INVALID_HANDLE_VALUE) {
        linebuf_destroy(&pane->buffer);
        return false;
    }

    pane->read_pos = 0;
    pane->following = true;
    pane->view_line = 0;
    pane->partial_line = NULL;
    pane->partial_len = 0;
    pane->top_row = 0;
    pane->height = 1;
    pane->content_height = 0;
    pane->dirty = true;

    return true;
}

void pane_destroy(TailPane *pane) {
    if (!pane) {
        return;
    }

    if (pane->file_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(pane->file_handle);
        pane->file_handle = INVALID_HANDLE_VALUE;
    }

    linebuf_destroy(&pane->buffer);
    free(pane->partial_line);
    pane->partial_line = NULL;
}

void pane_update(TailPane *pane) {
    if (!pane || pane->file_handle == INVALID_HANDLE_VALUE) {
        return;
    }

    // Get current file size
    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(pane->file_handle, &file_size)) {
        return;
    }

    // Check if file was truncated
    if (file_size.QuadPart < pane->read_pos) {
        pane->read_pos = 0;
        linebuf_clear(&pane->buffer);
        free(pane->partial_line);
        pane->partial_line = NULL;
        pane->partial_len = 0;
        pane->view_line = 0;
        pane->dirty = true;
    }

    // Check if there's new content
    if (file_size.QuadPart <= pane->read_pos) {
        return;
    }

    // Seek to read position
    LARGE_INTEGER pos;
    pos.QuadPart = pane->read_pos;
    if (!SetFilePointerEx(pane->file_handle, pos, NULL, FILE_BEGIN)) {
        return;
    }

    // Read new content
    char read_buf[READ_BUFFER_SIZE];
    DWORD bytes_read;

    while (pane->read_pos < file_size.QuadPart) {
        if (!ReadFile(pane->file_handle, read_buf, READ_BUFFER_SIZE, &bytes_read, NULL)) {
            break;
        }
        if (bytes_read == 0) {
            break;
        }

        process_read_data(pane, read_buf, bytes_read);
        pane->read_pos += bytes_read;
        pane->dirty = true;
    }
}

static void process_read_data(TailPane *pane, const char *data, DWORD len) {
    size_t start = 0;

    for (size_t i = 0; i < len; i++) {
        if (data[i] == '\n' || data[i] == '\r') {
            size_t line_len = i - start;

            // Build complete line
            size_t total_len = pane->partial_len + line_len;
            char *line = (char *)malloc(total_len + 1);
            if (line) {
                if (pane->partial_line && pane->partial_len > 0) {
                    memcpy(line, pane->partial_line, pane->partial_len);
                }
                if (line_len > 0) {
                    memcpy(line + pane->partial_len, data + start, line_len);
                }
                line[total_len] = '\0';

                linebuf_push(&pane->buffer, line);
                free(line);
            }

            // Clear partial line
            free(pane->partial_line);
            pane->partial_line = NULL;
            pane->partial_len = 0;

            // Skip \r\n sequence
            if (data[i] == '\r' && i + 1 < len && data[i + 1] == '\n') {
                i++;
            }
            start = i + 1;
        }
    }

    // Handle remaining partial line
    if (start < len) {
        size_t remaining = len - start;
        size_t new_len = pane->partial_len + remaining;
        char *new_partial = (char *)realloc(pane->partial_line, new_len + 1);
        if (new_partial) {
            memcpy(new_partial + pane->partial_len, data + start, remaining);
            new_partial[new_len] = '\0';
            pane->partial_line = new_partial;
            pane->partial_len = new_len;
        }
    }
}

void pane_render(TailPane *pane, Console *con, bool is_active) {
    if (!pane || !con) {
        return;
    }

    // Render header
    char header[256];
    const char *basename = strrchr(pane->filepath, '\\');
    if (!basename) {
        basename = strrchr(pane->filepath, '/');
    }
    basename = basename ? basename + 1 : pane->filepath;

    snprintf(header, sizeof(header), " [%s]%s", basename, is_active ? " *" : "");

    WORD header_attr = is_active ? COLOR_HEADER_ACTIVE : COLOR_HEADER;
    console_fill_row(con, pane->top_row, ' ', header_attr);
    console_write_at(con, pane->top_row, 0, header, header_attr);

    // Calculate view position for following mode
    size_t line_count = linebuf_count(&pane->buffer);
    if (pane->following && line_count > 0) {
        if (line_count > (size_t)pane->content_height) {
            pane->view_line = line_count - pane->content_height;
        } else {
            pane->view_line = 0;
        }
    }

    // Render content lines
    for (int i = 0; i < pane->content_height; i++) {
        int console_row = pane->top_row + 1 + i;
        size_t line_index = pane->view_line + i;

        const char *line = linebuf_get(&pane->buffer, line_index);
        console_write_fixed(con, console_row, 0, line, con->width, COLOR_DEFAULT);
    }

    pane->dirty = false;
}

bool pane_scroll_up(TailPane *pane) {
    if (!pane || pane->view_line == 0) {
        return false;
    }

    pane->view_line--;
    pane->following = false;
    pane->dirty = true;
    return true;
}

bool pane_scroll_down(TailPane *pane) {
    if (!pane) {
        return false;
    }

    size_t line_count = linebuf_count(&pane->buffer);
    size_t max_view = 0;
    if (line_count > (size_t)pane->content_height) {
        max_view = line_count - pane->content_height;
    }

    if (pane->view_line >= max_view) {
        // At the end - resume following
        pane->following = true;
        return false;
    }

    pane->view_line++;
    pane->dirty = true;
    return true;
}

void pane_page_up(TailPane *pane) {
    if (!pane) {
        return;
    }

    int page = pane->content_height > 0 ? pane->content_height : 1;
    if (pane->view_line >= (size_t)page) {
        pane->view_line -= page;
    } else {
        pane->view_line = 0;
    }
    pane->following = false;
    pane->dirty = true;
}

void pane_page_down(TailPane *pane) {
    if (!pane) {
        return;
    }

    size_t line_count = linebuf_count(&pane->buffer);
    size_t max_view = 0;
    if (line_count > (size_t)pane->content_height) {
        max_view = line_count - pane->content_height;
    }

    int page = pane->content_height > 0 ? pane->content_height : 1;
    pane->view_line += page;

    if (pane->view_line >= max_view) {
        pane->view_line = max_view;
        pane->following = true;
    }
    pane->dirty = true;
}

void pane_scroll_home(TailPane *pane) {
    if (!pane) {
        return;
    }
    pane->view_line = 0;
    pane->following = false;
    pane->dirty = true;
}

void pane_scroll_end(TailPane *pane) {
    if (!pane) {
        return;
    }

    size_t line_count = linebuf_count(&pane->buffer);
    if (line_count > (size_t)pane->content_height) {
        pane->view_line = line_count - pane->content_height;
    } else {
        pane->view_line = 0;
    }
    pane->following = true;
    pane->dirty = true;
}

void pane_set_region(TailPane *pane, int top_row, int height) {
    if (!pane) {
        return;
    }
    pane->top_row = top_row;
    pane->height = height;
    pane->content_height = height > 1 ? height - 1 : 0;  // -1 for header
    pane->dirty = true;
}
