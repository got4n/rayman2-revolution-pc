#ifndef SNDIOPBUF_INV_H
#define SNDIOPBUF_INV_H

typedef struct {
    unsigned int size;
    char *data;
} Span_inv;

typedef struct {
    unsigned int wr;
    unsigned int written;
    unsigned int limit;
    unsigned int rd;
    unsigned int read;
    Span_inv span;
} tdstRingBuffer_inv;

typedef struct {
    char *base;
    char *end;
    char *rd;
    char *wr;
    unsigned int count;
} tdstFifo_inv;

static inline unsigned int FifoSize_inv(tdstFifo_inv *r)
{
    return r->end - r->base;
}

static inline unsigned int FifoFree_inv(tdstFifo_inv *r)
{
    return r->end - r->base - r->count;
}

#endif
