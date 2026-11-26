#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "glue_rt.h"
#include "runtime.h"

FILE *revo_fopen(const char *path, const char *mode) {
#ifdef _WIN32
    wchar_t wpath[1024], wmode[8];
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, wpath, 1024) > 0 &&
        MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, 8) > 0)
        return _wfopen(wpath, wmode);
#endif
    return fopen(path, mode);
}

const char *revo_str(u32 addr) {
    u8 *p = revo_ptr(addr);
    return p ? (const char *)p : "";
}

typedef struct {
    u64 at, src;
    u32 size;
    char *path;
} BfPiece;

enum { DISC_HOST, DISC_MEMORY, DISC_PIECES };

typedef struct {
    char name[64];
    char path[1024];
    u64 base;
    u32 lsn, size;
    int kind;
    u8 *data;
    BfPiece *pieces;
    int npieces;
} DiscFile;
static DiscFile g_disc[512];
static int g_ndisc;
char revo_assets_dir[1024] = ".";
char revo_disc_image[1024];
char revo_bf_dir_default[1024];
char revo_sound_dir_default[1024];
char revo_save_dir[1024] = "./save";
static char g_bf_dir[1024];

static void scan_dir(const char *dir, const char *prefix, u32 *lsn) {
    DIR *d = opendir(dir);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) && g_ndisc < 512) {
        if (e->d_name[0] == '.') continue;
        char full[1024], nm[64];
        snprintf(full, sizeof full, "%s/%s", dir, e->d_name);
        if (!strcasecmp(full, g_bf_dir)) continue;
        snprintf(nm, sizeof nm, "%s%s", prefix, e->d_name);
        for (char *c = nm; *c; c++) if (*c >= 'a' && *c <= 'z') *c -= 32;
        struct stat st;
        if (stat(full, &st)) continue;
        if (S_ISDIR(st.st_mode)) {
            if (!strcmp(prefix, "SOUND\\")) continue;
            char pre[64];
            snprintf(pre, sizeof pre, "%s\\", nm);
            scan_dir(full, pre, lsn);
            continue;
        }
        if (strstr(nm, ".ZIP") || strstr(nm, ".Z01")) continue;
        DiscFile *f = &g_disc[g_ndisc++];
        memset(f, 0, sizeof *f);
        snprintf(f->name, sizeof f->name, "%s", nm);
        snprintf(f->path, sizeof f->path, "%s", full);
        f->lsn = *lsn;
        f->size = (u32)st.st_size;
        *lsn += (f->size + 2047) / 2048 + 16;
    }
    closedir(d);
}

static u32 le32(const u8 *p) { return p[0] | p[1] << 8 | p[2] << 16 | (u32)p[3] << 24; }

static int iso_sector(FILE *fp, u32 sector, u8 *dst) {
#ifdef _WIN32
    if (_fseeki64(fp, (long long)sector * 2048, SEEK_SET)) return 0;
#else
    if (fseeko(fp, (off_t)sector * 2048, SEEK_SET)) return 0;
#endif
    return fread(dst, 1, 2048, fp) == 2048;
}

static void iso_dir(FILE *fp, const char *iso, u32 extent, u32 size, const char *prefix, u32 *lsn, int depth) {
    if (depth > 8) return;
    u8 sec[2048];
    for (u32 off = 0; off < size; off += 2048) {
        if (!iso_sector(fp, extent + off / 2048, sec)) return;
        for (u32 at = 0; at < 2048 && sec[at];) {
            const u8 *r = sec + at;
            const u32 len = r[0], name_len = r[32];
            if (len < 34 || at + len > 2048 || 33 + name_len > len) break;
            at += len;
            if (name_len == 1 && (r[33] == 0 || r[33] == 1)) continue;
            char nm[64];
            snprintf(nm, sizeof nm, "%s%.*s", prefix, (int)name_len, (const char *)r + 33);
            char *semi = strchr(nm, ';');
            if (semi) *semi = 0;
            size_t n = strlen(nm);
            if (n && nm[n - 1] == '.') nm[n - 1] = 0;
            for (char *c = nm; *c; c++) if (*c >= 'a' && *c <= 'z') *c -= 32;
            if (r[25] & 2) {
                char pre[64];
                snprintf(pre, sizeof pre, "%s\\", nm);
                iso_dir(fp, iso, le32(r + 2), le32(r + 10), pre, lsn, depth + 1);
                continue;
            }
            if (g_ndisc >= 512) return;
            DiscFile *f = &g_disc[g_ndisc++];
            memset(f, 0, sizeof *f);
            snprintf(f->name, sizeof f->name, "%s", nm);
            snprintf(f->path, sizeof f->path, "%s", iso);
            f->base = (u64)le32(r + 2) * 2048;
            f->size = le32(r + 10);
            f->lsn = *lsn;
            *lsn += (f->size + 2047) / 2048 + 16;
        }
    }
}

static int iso_scan(const char *iso, u32 *lsn) {
    FILE *fp = revo_fopen(iso, "rb");
    if (!fp) return 0;
    u8 pvd[2048];
    int ok = iso_sector(fp, 16, pvd) && pvd[0] == 1 && !memcmp(pvd + 1, "CD001", 5);
    if (ok) iso_dir(fp, iso, le32(pvd + 156 + 2), le32(pvd + 156 + 10), "", lsn, 0);
    fclose(fp);
    return ok;
}

static int host_read(const char *path, u64 at, u32 n, u8 *dst) {
    FILE *fp = revo_fopen(path, "rb");
    if (!fp) return 0;
#ifdef _WIN32
    int ok = !_fseeki64(fp, (long long)at, SEEK_SET);
#else
    int ok = !fseeko(fp, (off_t)at, SEEK_SET);
#endif
    ok = ok && fread(dst, 1, n, fp) == n;
    fclose(fp);
    return ok;
}

static int file_read(const DiscFile *f, u64 at, u32 n, u8 *dst) {
    memset(dst, 0, n);
    if (at >= f->size) return 1;
    if (n > f->size - at) n = (u32)(f->size - at);
    if (f->kind == DISC_HOST) return host_read(f->path, f->base + at, n, dst);
    if (f->kind == DISC_MEMORY) { memcpy(dst, f->data + at, n); return 1; }
    int lo = 0, hi = f->npieces;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        if (f->pieces[mid].at + f->pieces[mid].size <= at) lo = mid + 1; else hi = mid;
    }
    for (int i = lo; i < f->npieces; i++) {
        const BfPiece *p = &f->pieces[i];
        if (p->at >= at + n) break;
        u64 from = p->at > at ? p->at : at;
        u64 to = p->at + p->size < at + n ? p->at + p->size : at + n;
        if (!host_read(p->path, p->src + (from - p->at), (u32)(to - from), dst + (from - at))) return 0;
    }
    return 1;
}

typedef struct {
    char key[256];
    char path[1024];
    u32 size;
    int used;
} BfFile;
static BfFile *g_bff;
static int g_nbff, g_capbff;

static void bf_scan(const char *dir, const char *prefix) {
    DIR *d = opendir(dir);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (e->d_name[0] == '.') continue;
        char full[1024], rel[256];
        snprintf(full, sizeof full, "%s/%s", dir, e->d_name);
        snprintf(rel, sizeof rel, "%s%s", prefix, e->d_name);
        for (char *c = rel; *c; c++) if (*c >= 'A' && *c <= 'Z') *c += 32;
        struct stat st;
        if (stat(full, &st)) continue;
        if (S_ISDIR(st.st_mode)) {
            char sub[256];
            snprintf(sub, sizeof sub, "%s/", rel);
            bf_scan(full, sub);
            continue;
        }
        if (strpbrk(e->d_name, "()[],")) { revo_log("[disc] %s: no such name in a bigfile, left out", full); continue; }
        if (g_nbff == g_capbff) {
            g_capbff = g_capbff * 2 + 256;
            g_bff = realloc(g_bff, sizeof *g_bff * (size_t)g_capbff);
        }
        BfFile *b = &g_bff[g_nbff++];
        snprintf(b->key, sizeof b->key, "%s", rel);
        snprintf(b->path, sizeof b->path, "%s", full);
        b->size = (u32)st.st_size;
        b->used = 0;
    }
    closedir(d);
}

static BfFile *bf_find(const char *key) {
    for (int i = 0; i < g_nbff; i++) if (!g_bff[i].used && !strcmp(g_bff[i].key, key)) return &g_bff[i];
    return NULL;
}

typedef struct {
    char *dsc;
    size_t len, cap;
    BfPiece *pieces;
    int n, cap_pieces;
    u64 end;
    int from_dir, from_bf, added, missing;
    const char *folder, *dsc_name;
} BfBuild;

static void put(BfBuild *b, const char *s, size_t n) {
    if (b->len + n + 1 > b->cap) {
        b->cap = (b->len + n + 1) * 2;
        b->dsc = realloc(b->dsc, b->cap);
    }
    memcpy(b->dsc + b->len, s, n);
    b->len += n;
    b->dsc[b->len] = 0;
}

static void add_file(BfBuild *b, const char *name, size_t nlen, const char *path, u64 src, u32 size, const char *flag) {
    if (b->n == b->cap_pieces) {
        b->cap_pieces = b->cap_pieces * 2 + 256;
        b->pieces = realloc(b->pieces, sizeof *b->pieces * (size_t)b->cap_pieces);
    }
    BfPiece *p = &b->pieces[b->n++];
    p->at = b->end;
    p->src = src;
    p->size = size;
    p->path = strdup(path);
    char entry[64];
    put(b, "[", 1);
    put(b, name, nlen);
    int n = snprintf(entry, sizeof entry, ",%llu,%u,%s]", (unsigned long long)b->end, size, flag);
    put(b, entry, (size_t)n);
    b->end += ((u64)size + 2047) / 2048 * 2048;
}

static void add_new(BfBuild *b, const char *dir) {
    size_t dl = strlen(dir);
    for (int i = 0; i < g_nbff; i++) {
        BfFile *f = &g_bff[i];
        if (f->used || strncmp(f->key, dir, dl) || strchr(f->key + dl, '/')) continue;
        f->used = 1;
        b->added++;
        add_file(b, f->key + dl, strlen(f->key + dl), f->path, 0, f->size, "0");
    }
    for (int i = 0; i < g_nbff; i++) {
        BfFile *f = &g_bff[i];
        if (f->used || strncmp(f->key, dir, dl)) continue;
        const char *slash = strchr(f->key + dl, '/');
        char sub[256];
        snprintf(sub, sizeof sub, "%.*s", (int)(slash + 1 - f->key), f->key);
        put(b, "(", 1);
        put(b, f->key + dl, (size_t)(slash - (f->key + dl)));
        add_new(b, sub);
        put(b, ")", 1);
    }
}

static u8 *read_whole(const DiscFile *f, size_t *size) {
    u8 *data = malloc((size_t)f->size + 1);
    if (!data) return NULL;
    if (!file_read(f, 0, f->size, data)) { free(data); return NULL; }
    data[f->size] = 0;
    *size = f->size;
    return data;
}

static DiscFile *disc_find(const char *gname);

static void bf_build(BfBuild *b, const char *dsc, const char *bf, u64 bf_base) {
    char dir[256] = "";
    size_t stack[32];
    int depth = 0;
    for (const char *s = dsc; *s;) {
        if (*s == '(') {
            const char *e = s + 1;
            while (*e && !strchr("([)", *e)) e++;
            put(b, s, (size_t)(e - s));
            if (depth > 0) {
                size_t dl = strlen(dir);
                snprintf(dir + dl, sizeof dir - dl, "%.*s/", (int)(e - s - 1), s + 1);
                for (char *c = dir + dl; *c; c++) if (*c >= 'A' && *c <= 'Z') *c += 32;
                stack[depth] = dl;
            }
            depth++;
            s = e;
        } else if (*s == ')') {
            add_new(b, dir);
            put(b, ")", 1);
            if (--depth > 0) dir[stack[depth]] = 0;
            s++;
            if (depth == 0) break;
        } else if (*s == '[') {
            const char *e = strchr(s, ']');
            if (!e) break;
            const char *c3 = e, *c2, *c1;
            while (c3 > s && *c3 != ',') c3--;
            c2 = c3 - 1;
            while (c2 > s && *c2 != ',') c2--;
            c1 = c2 - 1;
            while (c1 > s && *c1 != ',') c1--;
            if (c1 <= s) { s = e + 1; continue; }
            char key[256], flag[16];
            snprintf(key, sizeof key, "%s%.*s", dir, (int)(c1 - s - 1), s + 1);
            for (char *c = key; *c; c++) if (*c >= 'A' && *c <= 'Z') *c += 32;
            snprintf(flag, sizeof flag, "%.*s", (int)(e - c3 - 1), c3 + 1);
            BfFile *f = bf_find(key);
            if (f) {
                f->used = 1;
                b->from_dir++;
                add_file(b, s + 1, (size_t)(c1 - s - 1), f->path, 0, f->size, flag);
            } else if (bf) {
                b->from_bf++;
                add_file(b, s + 1, (size_t)(c1 - s - 1), bf, bf_base + strtoull(c1 + 1, NULL, 10),
                         (u32)strtoul(c2 + 1, NULL, 10), flag);
            } else {
                b->missing++;
                revo_log("[disc] %s: in %s, not in %s", key, b->dsc_name, b->folder);
            }
            s = e + 1;
        } else {
            put(b, s, 1);
            s++;
        }
    }
}

/* A bigfile (GAME.BF, or one of SOUND's) served from the folder its files were taken out to: the
   folder's files first, the bigfile's for the ones it lacks, and its .DSC made again. */
static void bf_from_folder(const char *folder, const char *bf_name, const char *dsc_name, u32 *lsn) {
    const char *env = getenv("REVO_BF_DIR");
    if (env && !strcmp(env, "off")) return;
    struct stat st;
    if (stat(folder, &st) || !S_ISDIR(st.st_mode)) return;
    g_nbff = 0;
    bf_scan(folder, "");
    if (!g_nbff) return;

    DiscFile *bf = disc_find(bf_name), *dscf = disc_find(dsc_name);
    size_t dsc_size = 0;
    u8 *dsc = dscf ? read_whole(dscf, &dsc_size) : NULL;
    BfBuild b = {0};
    b.folder = folder;
    b.dsc_name = dsc_name;
    bf_build(&b, dsc ? (const char *)dsc : "(GAME)", bf ? bf->path : NULL, bf ? bf->base : 0);
    free(dsc);
    if (b.end > 0x7fffffffu) {
        revo_log("[disc] %s: %llu bytes, more than a bigfile holds (2 GiB); %s kept",
                 folder, (unsigned long long)b.end, bf_name);
        return;
    }
    if (!bf || !dscf) {
        if (g_ndisc + 2 > (int)(sizeof g_disc / sizeof *g_disc)) return;
        if (!bf) bf = &g_disc[g_ndisc++];
        if (!dscf) dscf = &g_disc[g_ndisc++];
    }
    memset(bf, 0, sizeof *bf);
    snprintf(bf->name, sizeof bf->name, "%s", bf_name);
    snprintf(bf->path, sizeof bf->path, "%s", folder);
    bf->kind = DISC_PIECES;
    bf->pieces = b.pieces;
    bf->npieces = b.n;
    bf->size = (u32)b.end;
    bf->lsn = *lsn;
    *lsn += (bf->size + 2047) / 2048 + 16;
    memset(dscf, 0, sizeof *dscf);
    snprintf(dscf->name, sizeof dscf->name, "%s", dsc_name);
    snprintf(dscf->path, sizeof dscf->path, "%s (made again)", folder);
    dscf->kind = DISC_MEMORY;
    dscf->data = (u8 *)b.dsc;
    dscf->size = (u32)b.len;
    dscf->lsn = *lsn;
    *lsn += (dscf->size + 2047) / 2048 + 16;
    revo_log("[disc] %s from %s: %d files from the folder, %d new, %d from %s, %d missing",
             bf_name, folder, b.from_dir, b.added, b.from_bf, bf_name, b.missing);
}

/* SOUND's bigfiles, each from a folder of its name (SOUND/SNDENG for SNDENG.BF): next to them for
   the disc's files, or where the game's files were taken out to. */
static void sound_from_folders(u32 *lsn) {
    const int n = g_ndisc;
    for (int i = 0; i < n; i++) {
        const char *name = g_disc[i].name;
        const size_t len = strlen(name);
        if (strncmp(name, "SOUND\\", 6) || len < 10 || strcmp(name + len - 3, ".BF")) continue;
        char bf_name[64], dsc_name[64], folder[1100];
        snprintf(bf_name, sizeof bf_name, "%s", name);
        snprintf(dsc_name, sizeof dsc_name, "%.*s.DSC", (int)(len - 3), name);
        const char *base = name + 6;
        const int base_len = (int)(len - 9);
        struct stat st;
        snprintf(folder, sizeof folder, "%s/SOUND/%.*s", revo_assets_dir, base_len, base);
        if (revo_disc_image[0] || stat(folder, &st) || !S_ISDIR(st.st_mode)) {
            if (!revo_sound_dir_default[0]) continue;
            snprintf(folder, sizeof folder, "%s/%.*s", revo_sound_dir_default, base_len, base);
        }
        bf_from_folder(folder, bf_name, dsc_name, lsn);
    }
}

const char *revo_disc_file(int i, u32 *size) {
    if (i < 0 || i >= g_ndisc) return NULL;
    if (size) *size = g_disc[i].size;
    return g_disc[i].name;
}

int revo_disc_open(const char *source) {
    u32 lsn = 1000;
    g_ndisc = 0;
    g_nbff = 0;
    revo_disc_image[0] = 0;
    struct stat st;
    FILE *probe = revo_fopen(source, "rb");
    const int image = probe && (stat(source, &st) != 0 || !S_ISDIR(st.st_mode));
    if (probe) fclose(probe);
    if (image) {
        if (!iso_scan(source, &lsn)) { g_ndisc = 0; return 0; }
        snprintf(revo_disc_image, sizeof revo_disc_image, "%s", source);
        snprintf(revo_assets_dir, sizeof revo_assets_dir, "%s", source);
        char *slash = strrchr(revo_assets_dir, '/'), *back = strrchr(revo_assets_dir, '\\');
        if (back > slash) slash = back;
        if (slash) *slash = 0; else snprintf(revo_assets_dir, sizeof revo_assets_dir, ".");
        g_bf_dir[0] = 0;
    } else {
        snprintf(revo_assets_dir, sizeof revo_assets_dir, "%s", source);
        snprintf(g_bf_dir, sizeof g_bf_dir, "%s/GAME", revo_assets_dir);
        scan_dir(revo_assets_dir, "", &lsn);
    }
    if (!disc_find(revo_full_program() ? "GAME.DSC" : "SLUS_201.38")) { g_ndisc = 0; return 0; }
    struct stat bst;
    if (revo_bf_dir_default[0] && (!g_bf_dir[0] || stat(g_bf_dir, &bst) || !S_ISDIR(bst.st_mode)))
        snprintf(g_bf_dir, sizeof g_bf_dir, "%s", revo_bf_dir_default);
    const char *env = getenv("REVO_BF_DIR");
    if (env && *env && strcmp(env, "off")) snprintf(g_bf_dir, sizeof g_bf_dir, "%s", env);
    if (g_bf_dir[0]) bf_from_folder(g_bf_dir, "GAME.BF", "GAME.DSC", &lsn);
    sound_from_folders(&lsn);
    revo_log("[disc] %d files from %s", g_ndisc, revo_disc_image[0] ? revo_disc_image : revo_assets_dir);
    return 1;
}

int revo_disc_read_file(const char *name, u64 at, u32 size, u8 *dst) {
    const DiscFile *f = disc_find(name);
    return f && at + size <= f->size && file_read(f, at, size, dst);
}

u8 *revo_disc_load(const char *name, u32 *size) {
    DiscFile *f = disc_find(name);
    size_t n = 0;
    u8 *data = f ? read_whole(f, &n) : NULL;
    if (size) *size = (u32)n;
    return data;
}

static void disc_name(const char *gname, char *nm, size_t size) {
    const char *s = gname;
    if (!strncasecmp(s, "cdrom0:", 7)) s += 7;
    while (*s == '\\' || *s == '/') s++;
    snprintf(nm, size, "%s", s);
    char *semi = strchr(nm, ';');
    if (semi) *semi = 0;
    for (char *c = nm; *c; c++) { if (*c == '/') *c = '\\'; if (*c >= 'a' && *c <= 'z') *c -= 32; }
}

static DiscFile *disc_find(const char *gname) {
    char nm[128];
    disc_name(gname, nm, sizeof nm);
    for (int i = 0; i < g_ndisc; i++) if (!strcmp(g_disc[i].name, nm)) return &g_disc[i];
    return NULL;
}

int revo_disc_find(const char *name, u32 *lsn, u32 *size) {
    DiscFile *f = disc_find(name);
    if (!f) { revo_log("[disc] \"%s\": not found", name); return 0; }
    *lsn = f->lsn;
    *size = f->size;
    return 1;
}

static DiscFile *disc_at(u32 lsn) {
    for (int i = 0; i < g_ndisc; i++) {
        DiscFile *f = &g_disc[i];
        u32 sectors = (f->size + 2047) / 2048;
        if (lsn >= f->lsn && lsn < f->lsn + sectors + 16) return f;
    }
    return NULL;
}

int revo_disc_read_bytes(u32 lsn, u32 offset, u32 size, u8 *dst) {
    DiscFile *f = disc_at(lsn);
    if (!f) { revo_log("[disc] no file has sector %u", lsn); return 0; }
    return file_read(f, (u64)(lsn - f->lsn) * 2048 + offset, size, dst);
}

int revo_disc_read(u32 lsn, u32 sectors, u8 *dst) {
    DiscFile *f = disc_at(lsn);
    if (!f) { revo_log("[disc] read of unknown sector %u", lsn); return 0; }
    return file_read(f, (u64)(lsn - f->lsn) * 2048, sectors * 2048, dst);
}
