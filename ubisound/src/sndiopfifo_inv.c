#include "include_asm.h"
#include "sndiopbuf_inv.h"

extern void *sysclib_12_inv(void *dst, void *src, int n);

static inline unsigned int FifoCount_inv(tdstFifo_inv *r)
{
    return r->count;
}

void fn_vWriteTdstFifo_inv(tdstFifo_inv *r, char *src, unsigned int n)
{
    unsigned int avail;

    if (n != 0) {
        avail = r->end - r->wr;
        if (n <= avail) {
#ifdef PC_PORT
            pc_memcpy(r->wr, src, n);
#else
            sysclib_12_inv(r->wr, src, n);
#endif
            r->wr += n;
            r->count += n;
        } else {
            fn_vWriteTdstFifo_inv(r, src, avail);
            r->wr = r->base;
            fn_vWriteTdstFifo_inv(r, src + avail, n - avail);
        }
    }
}

void fn_vReadTdstFifo_inv(tdstFifo_inv *r, char *dst, unsigned int n)
{
    unsigned int avail;

    if (n != 0 && r->count >= n) {
        avail = r->end - r->rd;
        if (n <= avail) {
#ifdef PC_PORT
            pc_memcpy(dst, r->rd, n);
#else
            sysclib_12_inv(dst, r->rd, n);
#endif
            r->rd += n;
            r->count -= n;
        } else {
            fn_vReadTdstFifo_inv(r, dst, avail);
            r->rd = r->base;
            fn_vReadTdstFifo_inv(r, dst + avail, n - avail);
        }
    }
}

unsigned int fn_u32PushTdstFifo_inv(tdstFifo_inv *r, Span_inv *s)
{
    unsigned int n = s->size < FifoFree_inv(r) ? s->size : FifoFree_inv(r);

    fn_vWriteTdstFifo_inv(r, s->data, n);
    return n;
}

unsigned int fn_u32PopTdstFifo_inv(tdstFifo_inv *r, Span_inv *s)
{
    unsigned int n = s->size < FifoCount_inv(r) ? s->size : FifoCount_inv(r);

    fn_vReadTdstFifo_inv(r, s->data, n);
    return n;
}
