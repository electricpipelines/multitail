#include "linebuf.h"
#include <stdlib.h>
#include <string.h>

bool linebuf_init(LineBuffer *buf, size_t capacity) {
    if (!buf || capacity == 0) {
        return false;
    }

    buf->lines = (char **)calloc(capacity, sizeof(char *));
    if (!buf->lines) {
        return false;
    }

    buf->capacity = capacity;
    buf->count = 0;
    buf->head = 0;
    return true;
}

void linebuf_destroy(LineBuffer *buf) {
    if (!buf || !buf->lines) {
        return;
    }

    for (size_t i = 0; i < buf->capacity; i++) {
        free(buf->lines[i]);
    }
    free(buf->lines);

    buf->lines = NULL;
    buf->capacity = 0;
    buf->count = 0;
    buf->head = 0;
}

void linebuf_clear(LineBuffer *buf) {
    if (!buf || !buf->lines) {
        return;
    }

    for (size_t i = 0; i < buf->capacity; i++) {
        free(buf->lines[i]);
        buf->lines[i] = NULL;
    }

    buf->count = 0;
    buf->head = 0;
}

bool linebuf_push(LineBuffer *buf, const char *line) {
    if (!buf || !buf->lines || !line) {
        return false;
    }

    // Calculate the physical index where we'll store the new line
    size_t physical_index;
    if (buf->count < buf->capacity) {
        // Buffer not full yet - append at the end
        physical_index = (buf->head + buf->count) % buf->capacity;
        buf->count++;
    } else {
        // Buffer full - overwrite oldest line and advance head
        physical_index = buf->head;
        free(buf->lines[physical_index]);
        buf->head = (buf->head + 1) % buf->capacity;
    }

    // Copy the line
    buf->lines[physical_index] = _strdup(line);
    if (!buf->lines[physical_index]) {
        return false;
    }

    return true;
}

const char *linebuf_get(const LineBuffer *buf, size_t index) {
    if (!buf || !buf->lines || index >= buf->count) {
        return NULL;
    }

    size_t physical_index = (buf->head + index) % buf->capacity;
    return buf->lines[physical_index];
}

size_t linebuf_count(const LineBuffer *buf) {
    if (!buf) {
        return 0;
    }
    return buf->count;
}
