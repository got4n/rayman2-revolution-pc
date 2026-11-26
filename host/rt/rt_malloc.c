#include "game_vars.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../crt.h"
#include "../../gli/guest.h"

#define AV GAME_VAR(__malloc_av_)
#define TRIM_THRESHOLD GAME_VAR(__malloc_trim_threshold)
#define TOP_PAD GAME_VAR(__malloc_top_pad)
#define SBRK_BASE GAME_VAR(__malloc_sbrk_base)
#define MAX_SBRKED GAME_VAR(__malloc_max_sbrked_mem)
#define MAX_TOTAL GAME_VAR(__malloc_max_total_mem)
#define MALLINFO GAME_VAR(__malloc_current_mallinfo)
#define IMPURE_PTR GAME_VAR(_impure_ptr)

#define SIZE_SZ 4u
#define ALIGNMENT 16u
#define ALIGN_MASK 15u
#define MINSIZE 16u
#define PREV_INUSE 1u
#define SIZE_BITS 3u
#define NAV 128
#define BINBLOCKWIDTH 4
#define MAX_SMALLBIN_SIZE 512u
#define SMALLBIN_WIDTH 8u
#define PAGESZ 4096u
#define ENOMEM 12

static inline u32 rd(u32 a) { return *GUEST(u32, a); }
static inline void wr(u32 a, u32 v) { *GUEST(u32, a) = v; }

static inline u32 size_of(u32 p) { return rd(p + 4); }
static inline u32 chunksize(u32 p) { return size_of(p) & ~SIZE_BITS; }
static inline void set_head(u32 p, u32 s) { wr(p + 4, s); }
static inline void set_head_size(u32 p, u32 s) { wr(p + 4, (size_of(p) & SIZE_BITS) | s); }
static inline void set_foot(u32 p, u32 s) { wr(p + s, s); }
static inline u32 fd(u32 p) { return rd(p + 8); }
static inline u32 bk(u32 p) { return rd(p + 12); }
static inline void set_fd(u32 p, u32 v) { wr(p + 8, v); }
static inline void set_bk(u32 p, u32 v) { wr(p + 12, v); }
static inline u32 mem2chunk(u32 m) { return m - 2 * SIZE_SZ; }
static inline u32 chunk2mem(u32 p) { return p + 2 * SIZE_SZ; }
static inline bool prev_inuse(u32 p) { return size_of(p) & PREV_INUSE; }
static inline bool inuse_at(u32 p, u32 s) { return size_of(p + s) & PREV_INUSE; }
static inline void set_inuse_at(u32 p, u32 s) { wr(p + s + 4, size_of(p + s) | PREV_INUSE); }

static inline u32 bin_at(int i) { return AV + 8u * (u32)i; }
static inline u32 top(void) { return fd(bin_at(0)); }
static inline void set_top(u32 p) { set_fd(bin_at(0), p); }
#define LAST_REMAINDER bin_at(1)
#define INITIAL_TOP bin_at(0)
static inline u32 binblocks(void) { return rd(AV + 4); }
static inline void set_binblocks(u32 v) { wr(AV + 4, v); }
static inline u32 idx2binblock(int i) { return 1u << (i / BINBLOCKWIDTH); }
static inline void mark_binblock(int i) { set_binblocks(binblocks() | idx2binblock(i)); }

static inline int bin_index(u32 sz) {
    u32 h = sz >> 9;
    if (h == 0) return (int)(sz >> 3);
    if (h <= 4) return 56 + (int)(sz >> 6);
    if (h <= 20) return 91 + (int)(sz >> 9);
    if (h <= 84) return 110 + (int)(sz >> 12);
    if (h <= 340) return 119 + (int)(sz >> 15);
    if (h <= 1364) return 124 + (int)(sz >> 18);
    return 126;
}

static inline void unlink_chunk(u32 p) {
    u32 b = bk(p), f = fd(p);
    set_bk(f, b);
    set_fd(b, f);
}
static inline void link_last_remainder(u32 p) {
    set_fd(LAST_REMAINDER, p);
    set_bk(LAST_REMAINDER, p);
    set_fd(p, LAST_REMAINDER);
    set_bk(p, LAST_REMAINDER);
}
static inline void clear_last_remainder(void) {
    set_fd(LAST_REMAINDER, LAST_REMAINDER);
    set_bk(LAST_REMAINDER, LAST_REMAINDER);
}
static void frontlink(u32 p, u32 s) {
    int idx;
    u32 b, f;
    if (s < MAX_SMALLBIN_SIZE) {
        idx = (int)(s >> 3);
        mark_binblock(idx);
        b = bin_at(idx);
        f = fd(b);
    } else {
        idx = bin_index(s);
        b = bin_at(idx);
        f = fd(b);
        if (f == b) mark_binblock(idx);
        else {
            while (f != b && s < chunksize(f)) f = fd(f);
            b = bk(f);
        }
    }
    set_bk(p, b);
    set_fd(p, f);
    set_bk(f, p);
    set_fd(b, p);
}

static inline u32 reent(void) { return rd(IMPURE_PTR); }
static inline void set_errno(u32 r, u32 e) { wr(r, e); }
static u32 sbrk_r(u32 r, s32 increment) { return crt_sbrk_r(r, increment); }

static u32 rt_malloc(u32 r, u32 bytes);
static void rt_free(u32 r, u32 mem);

static void extend_top(u32 r, u32 nb) {
    u32 old_top = top(), old_top_size = chunksize(old_top), old_end = old_top + old_top_size;
    u32 sbrk_size = nb + (u32)rd64(TOP_PAD) + MINSIZE;
    u32 base = rd(SBRK_BASE);
    if (base != 0xFFFFFFFFu) sbrk_size = (sbrk_size + (PAGESZ - 1)) & ~(PAGESZ - 1);
    u32 brk = sbrk_r(r, (s32)sbrk_size);
    if (brk == 0xFFFFFFFFu || (brk < old_end && old_top != INITIAL_TOP)) return;
    u32 sbrked = rd(MALLINFO) + sbrk_size;
    wr(MALLINFO, sbrked);
    if (brk == old_end) {
        set_head(top(), (sbrk_size + old_top_size) | PREV_INUSE);
    } else {
        u32 correction;
        if (base == 0xFFFFFFFFu) wr(SBRK_BASE, brk);
        else wr(MALLINFO, sbrked = sbrked + (brk - old_end));
        u32 misalign = chunk2mem(brk) & ALIGN_MASK;
        if (misalign) {
            correction = ALIGNMENT - misalign;
            brk += correction;
        } else correction = 0;
        correction += PAGESZ - ((brk + sbrk_size) & (PAGESZ - 1));
        u32 new_brk = sbrk_r(r, (s32)correction);
        if (new_brk == 0xFFFFFFFFu) return;
        wr(MALLINFO, sbrked = rd(MALLINFO) + correction);
        set_top(brk);
        set_head(brk, (new_brk - brk + correction) | PREV_INUSE);
        if (old_top != INITIAL_TOP) {
            if (old_top_size < MINSIZE) {
                set_head(brk, PREV_INUSE);
                return;
            }
            old_top_size = (old_top_size - 3 * SIZE_SZ) & ~ALIGN_MASK;
            set_head_size(old_top, old_top_size);
            wr(old_top + old_top_size + 4, SIZE_SZ | PREV_INUSE);
            wr(old_top + old_top_size + SIZE_SZ + 4, SIZE_SZ | PREV_INUSE);
            if (old_top_size >= MINSIZE) rt_free(r, chunk2mem(old_top));
        }
    }
    u64 total = rd(MALLINFO);
    if (total > rd64(MAX_SBRKED)) wr64(MAX_SBRKED, total);
    if (total > rd64(MAX_TOTAL)) wr64(MAX_TOTAL, total);
}

static int trim(u32 r, u64 pad) {
    s64 top_size = chunksize(top());
    s64 extra = (s64)((((u64)(top_size - (s64)pad - MINSIZE) + (PAGESZ - 1)) / PAGESZ - 1) * PAGESZ);
    if (extra < (s64)PAGESZ) return 0;
    u32 current = sbrk_r(r, 0);
    if (current != top() + (u32)top_size) return 0;
    u32 new_brk = sbrk_r(r, (s32)-extra);
    if (new_brk == 0xFFFFFFFFu) {
        current = sbrk_r(r, 0);
        top_size = (s64)(current - top());
        if (top_size >= (s64)MINSIZE) {
            wr(MALLINFO, current - rd(SBRK_BASE));
            set_head(top(), (u32)top_size | PREV_INUSE);
        }
        return 0;
    }
    set_head(top(), (u32)(top_size - extra) | PREV_INUSE);
    wr(MALLINFO, rd(MALLINFO) - (u32)extra);
    return 1;
}

static inline u32 request2size(u32 req) {
    return req + SIZE_SZ + ALIGN_MASK < MINSIZE + ALIGN_MASK ? MINSIZE : (req + SIZE_SZ + ALIGN_MASK) & ~ALIGN_MASK;
}

static u32 split_victim(u32 victim, u32 nb, s64 remainder_size) {
    u32 rem = victim + nb;
    set_head(victim, nb | PREV_INUSE);
    link_last_remainder(rem);
    set_head(rem, (u32)remainder_size | PREV_INUSE);
    set_foot(rem, (u32)remainder_size);
    return chunk2mem(victim);
}

static u32 rt_malloc(u32 r, u32 bytes) {
    u32 nb = request2size(bytes);
    if (nb > 0x7FFFFFFFu || nb < bytes) {
        set_errno(r, ENOMEM);
        return 0;
    }
    int idx;
    u32 victim, q;
    s64 remainder_size;
    if (nb < MAX_SMALLBIN_SIZE - SMALLBIN_WIDTH) {
        idx = (int)(nb >> 3);
        q = bin_at(idx);
        victim = bk(q);
        if (victim == q) {
            q = bin_at(idx + 1);
            victim = bk(q);
        }
        if (victim != q) {
            u32 vs = chunksize(victim);
            unlink_chunk(victim);
            set_inuse_at(victim, vs);
            return chunk2mem(victim);
        }
        idx += 2;
    } else {
        idx = bin_index(nb);
        u32 bin = bin_at(idx);
        for (victim = bk(bin); victim != bin; victim = bk(victim)) {
            u32 vs = chunksize(victim);
            remainder_size = (s64)vs - (s64)nb;
            if (remainder_size >= (s64)MINSIZE) {
                --idx;
                break;
            }
            if (remainder_size >= 0) {
                unlink_chunk(victim);
                set_inuse_at(victim, vs);
                return chunk2mem(victim);
            }
        }
        ++idx;
    }

    if ((victim = fd(LAST_REMAINDER)) != LAST_REMAINDER) {
        u32 vs = chunksize(victim);
        remainder_size = (s64)vs - (s64)nb;
        if (remainder_size >= (s64)MINSIZE) return split_victim(victim, nb, remainder_size);
        clear_last_remainder();
        if (remainder_size >= 0) {
            set_inuse_at(victim, vs);
            return chunk2mem(victim);
        }
        frontlink(victim, vs);
    }

    u32 block = idx2binblock(idx);
    if (block <= binblocks()) {
        if (!(block & binblocks())) {
            idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
            block <<= 1;
            while (!(block & binblocks())) {
                idx += BINBLOCKWIDTH;
                block <<= 1;
            }
        }
        for (;;) {
            int startidx = idx;
            u32 bin = bin_at(idx);
            q = bin;
            do {
                for (victim = bk(bin); victim != bin; victim = bk(victim)) {
                    u32 vs = chunksize(victim);
                    remainder_size = (s64)vs - (s64)nb;
                    if (remainder_size >= (s64)MINSIZE) {
                        u32 rem = victim + nb;
                        set_head(victim, nb | PREV_INUSE);
                        unlink_chunk(victim);
                        link_last_remainder(rem);
                        set_head(rem, (u32)remainder_size | PREV_INUSE);
                        set_foot(rem, (u32)remainder_size);
                        return chunk2mem(victim);
                    }
                    if (remainder_size >= 0) {
                        set_inuse_at(victim, vs);
                        unlink_chunk(victim);
                        return chunk2mem(victim);
                    }
                }
                bin += 8;
            } while ((++idx & (BINBLOCKWIDTH - 1)) != 0);
            do {
                if ((startidx & (BINBLOCKWIDTH - 1)) == 0) {
                    set_binblocks(binblocks() & ~block);
                    break;
                }
                --startidx;
                q -= 8;
            } while (fd(q) == q);
            if ((block <<= 1) <= binblocks() && block != 0) {
                while (!(block & binblocks())) {
                    idx += BINBLOCKWIDTH;
                    block <<= 1;
                }
            } else break;
        }
    }

    if ((remainder_size = (s64)chunksize(top()) - (s64)nb) < (s64)MINSIZE) {
        extend_top(r, nb);
        if ((remainder_size = (s64)chunksize(top()) - (s64)nb) < (s64)MINSIZE) return 0;
    }
    victim = top();
    set_head(victim, nb | PREV_INUSE);
    set_top(victim + nb);
    set_head(victim + nb, (u32)remainder_size | PREV_INUSE);
    return chunk2mem(victim);
}

static void rt_free(u32 r, u32 mem) {
    if (!mem) return;
    u32 p = mem2chunk(mem), hd = size_of(p), sz = hd & ~PREV_INUSE;
    u32 next = p + sz, nextsz = chunksize(next);
    if (next == top()) {
        sz += nextsz;
        if (!(hd & PREV_INUSE)) {
            u32 prevsz = rd(p);
            p -= prevsz;
            sz += prevsz;
            unlink_chunk(p);
        }
        set_head(p, sz | PREV_INUSE);
        set_top(p);
        if ((u64)sz >= rd64(TRIM_THRESHOLD)) trim(r, rd64(TOP_PAD));
        return;
    }
    set_head(next, nextsz);
    bool islr = false;
    if (!(hd & PREV_INUSE)) {
        u32 prevsz = rd(p);
        p -= prevsz;
        sz += prevsz;
        if (fd(p) == LAST_REMAINDER) islr = true;
        else unlink_chunk(p);
    }
    if (!inuse_at(next, nextsz)) {
        sz += nextsz;
        if (!islr && fd(next) == LAST_REMAINDER) {
            islr = true;
            link_last_remainder(p);
        } else unlink_chunk(next);
    }
    set_head(p, sz | PREV_INUSE);
    set_foot(p, sz);
    if (!islr) frontlink(p, sz);
}

static inline void copy(u32 dst, u32 src, u32 n) { memmove(GUEST(u8, dst), GUEST(u8, src), n); }

static u32 rt_realloc(u32 r, u32 oldmem, u32 bytes) {
    if (!oldmem) return rt_malloc(r, bytes);
    u32 oldp = mem2chunk(oldmem), newp = oldp;
    u32 oldsize = chunksize(oldp), newsize = oldsize;
    u32 nb = request2size(bytes);
    if (nb > 0x7FFFFFFFu || nb < bytes) {
        set_errno(r, ENOMEM);
        return 0;
    }
    if ((s64)oldsize < (s64)nb) {
        u32 next = oldp + oldsize, nextsize = 0;
        if (next == top() || !inuse_at(next, chunksize(next))) {
            nextsize = chunksize(next);
            if (next == top()) {
                if ((s64)(nextsize + newsize) >= (s64)(nb + MINSIZE)) {
                    newsize += nextsize;
                    set_top(oldp + nb);
                    set_head(oldp + nb, (newsize - nb) | PREV_INUSE);
                    set_head_size(oldp, nb);
                    return chunk2mem(oldp);
                }
            } else if ((s64)(nextsize + newsize) >= (s64)nb) {
                unlink_chunk(next);
                newsize += nextsize;
                goto split;
            }
        } else {
            next = 0;
            nextsize = 0;
        }
        if (!prev_inuse(oldp)) {
            u32 prev = oldp - rd(oldp), prevsize = chunksize(prev);
            if (next) {
                if (next == top()) {
                    if ((s64)(nextsize + prevsize + newsize) >= (s64)(nb + MINSIZE)) {
                        unlink_chunk(prev);
                        newp = prev;
                        newsize += prevsize + nextsize;
                        copy(chunk2mem(newp), oldmem, oldsize - SIZE_SZ);
                        set_top(newp + nb);
                        set_head(newp + nb, (newsize - nb) | PREV_INUSE);
                        set_head_size(newp, nb);
                        return chunk2mem(newp);
                    }
                } else if ((s64)(nextsize + prevsize + newsize) >= (s64)nb) {
                    unlink_chunk(next);
                    unlink_chunk(prev);
                    newp = prev;
                    newsize += nextsize + prevsize;
                    copy(chunk2mem(newp), oldmem, oldsize - SIZE_SZ);
                    goto split;
                }
            }
            if (prev && (s64)(prevsize + newsize) >= (s64)nb) {
                unlink_chunk(prev);
                newp = prev;
                newsize += prevsize;
                copy(chunk2mem(newp), oldmem, oldsize - SIZE_SZ);
                goto split;
            }
        }
        u32 newmem = rt_malloc(r, bytes);
        if (!newmem) return 0;
        if ((newp = mem2chunk(newmem)) == oldp + oldsize) {
            newsize += chunksize(newp);
            newp = oldp;
            goto split;
        }
        copy(newmem, oldmem, oldsize - SIZE_SZ);
        rt_free(r, oldmem);
        return newmem;
    }
split:;
    s64 remainder_size = (s64)newsize - (s64)nb;
    if (remainder_size >= (s64)MINSIZE) {
        u32 rem = newp + nb;
        set_head_size(newp, nb);
        set_head(rem, (u32)remainder_size | PREV_INUSE);
        set_inuse_at(rem, (u32)remainder_size);
        rt_free(r, chunk2mem(rem));
    } else {
        set_head_size(newp, newsize);
        set_inuse_at(newp, newsize);
    }
    return chunk2mem(newp);
}

static u32 rt_memalign(u32 r, u32 alignment, u32 bytes) {
    if (alignment <= ALIGNMENT) return rt_malloc(r, bytes);
    if (alignment < MINSIZE) alignment = MINSIZE;
    u32 nb = request2size(bytes);
    if (nb > 0x7FFFFFFFu || nb < bytes) {
        set_errno(r, ENOMEM);
        return 0;
    }
    u32 m = rt_malloc(r, nb + alignment + MINSIZE);
    if (!m) return 0;
    u32 p = mem2chunk(m);
    if (m % alignment) {
        u32 brk = mem2chunk((m + alignment - 1) & (0u - alignment));
        if ((s64)(brk - p) < (s64)MINSIZE) brk += alignment;
        u32 leadsize = brk - p, newsize = chunksize(p) - leadsize;
        set_head(brk, newsize | PREV_INUSE);
        set_inuse_at(brk, newsize);
        set_head_size(p, leadsize);
        rt_free(r, chunk2mem(p));
        p = brk;
    }
    s64 remainder_size = (s64)chunksize(p) - (s64)nb;
    if (remainder_size >= (s64)MINSIZE) {
        u32 rem = p + nb;
        set_head(rem, (u32)remainder_size | PREV_INUSE);
        set_head_size(p, nb);
        rt_free(r, chunk2mem(rem));
    }
    return chunk2mem(p);
}

u32 crt_malloc(u32 size) { return rt_malloc(reent(), size); }
void crt_freemem(u32 p) { rt_free(reent(), p); }
u32 crt_realloc(u32 p, u32 size) { return rt_realloc(reent(), p, size); }
u32 crt_memalign(u32 alignment, u32 size) { return rt_memalign(reent(), alignment, size); }
