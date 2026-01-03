#ifndef LINEBUF_H
#define LINEBUF_H

#include <stdbool.h>
#include <stddef.h>

#define LINEBUF_DEFAULT_CAPACITY 100000

typedef struct {
    char **lines;       // Circular buffer of line strings
    size_t capacity;    // Max lines to retain
    size_t count;       // Current number of lines stored
    size_t head;        // Index of oldest line (start of logical buffer)
} LineBuffer;

// Initialize a line buffer with given capacity
bool linebuf_init(LineBuffer *buf, size_t capacity);

// Free all resources
void linebuf_destroy(LineBuffer *buf);

// Clear all lines but keep capacity
void linebuf_clear(LineBuffer *buf);

// Add a line (makes a copy). Overwrites oldest if at capacity.
bool linebuf_push(LineBuffer *buf, const char *line);

// Get line at logical index (0 = oldest). Returns NULL if out of range.
const char *linebuf_get(const LineBuffer *buf, size_t index);

// Get current line count
size_t linebuf_count(const LineBuffer *buf);

#endif // LINEBUF_H
