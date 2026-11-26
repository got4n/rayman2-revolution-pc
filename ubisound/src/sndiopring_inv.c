#include "include_asm.h"
#include "sndiopbuf_inv.h"

extern void *sysclib_12_inv(void *dst, void *src, int n);

unsigned int fn_u32WriteTdstRingBuffer_inv(tdstRingBuffer_inv *r, Span_inv *req)
{
    Span_inv c;
    unsigned int done = 0;
    unsigned int n;

    while (done < req->size) {
        c.size = req->size - done;
        c.data = req->data + done;
        if (r->wr < r->rd)
            n = r->rd - r->wr;
        else
            n = r->span.size - r->wr;
        if (c.size < n)
            n = c.size;
        if (r->limit - r->written < n)
            n = r->limit - r->written;
        if (n == 0)
            break;
#ifdef PC_PORT
        pc_memcpy(r->span.data + r->wr, c.data, n);
#else
        sysclib_12_inv(r->span.data + r->wr, c.data, n);
#endif
        done += n;
        r->wr += n;
        r->written += n;
        if (r->wr == r->span.size)
            r->wr = 0;
    }
    return done;
}

unsigned int fn_u32ReadTdstRingBuffer_inv(tdstRingBuffer_inv *r, Span_inv *req)
{
    Span_inv c;
    unsigned int done = 0;
    unsigned int n;

    while (done < req->size) {
        c.size = req->size - done;
        c.data = req->data + done;
        n = r->written - r->read;
        if (c.size < n)
            n = c.size;
        if (n == 0)
            break;
#ifdef PC_PORT
        pc_memcpy(c.data, r->span.data + r->rd, n);
#else
        sysclib_12_inv(c.data, r->span.data + r->rd, n);
#endif
        done += n;
        r->rd += n;
        r->read += n;
        if (r->rd == r->span.size)
            r->rd = 0;
    }
    return done;
}
