#ifndef C_API_H
#define C_API_H

#include <stddef.h>

/* Returns the larger of a and b. */
int max_of(int a, int b);

int min_of(int a, int b);

/*
 * A growable byte buffer.
 */
struct buffer {
    char *data; /* the bytes */
    size_t size;
};

/** Frees the buffer. Safe to call twice. */
void buffer_free(struct buffer *b);

typedef struct {
    int code;
} status_t;

extern int error_count;

static int internal_helper(void) {
    return 0;
}

#endif
