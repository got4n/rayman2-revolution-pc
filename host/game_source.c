#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include <sys/stat.h>

#ifdef _WIN32
#define COBJMACROS
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#else
#include <unistd.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include "mods.h"
#include "runtime.h"

#define RETAIL_SHA1 "33618569b7a1aa23300e9d1139510220189b3abe"

typedef struct { u32 h[5]; u64 len; u8 buf[64]; u32 n; } Sha1;

static u32 rol(u32 x, int k) { return x << k | x >> (32 - k); }

static void sha1_block(Sha1 *s, const u8 *p) {
    u32 w[80];
    for (int i = 0; i < 16; i++) w[i] = (u32)p[4 * i] << 24 | p[4 * i + 1] << 16 | p[4 * i + 2] << 8 | p[4 * i + 3];
    for (int i = 16; i < 80; i++) w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    u32 a = s->h[0], b = s->h[1], c = s->h[2], d = s->h[3], e = s->h[4];
    for (int i = 0; i < 80; i++) {
        u32 f, k;
        if (i < 20) { f = (b & c) | (~b & d); k = 0x5A827999; }
        else if (i < 40) { f = b ^ c ^ d; k = 0x6ED9EBA1; }
        else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
        else { f = b ^ c ^ d; k = 0xCA62C1D6; }
        u32 t = rol(a, 5) + f + e + k + w[i];
        e = d; d = c; c = rol(b, 30); b = a; a = t;
    }
    s->h[0] += a; s->h[1] += b; s->h[2] += c; s->h[3] += d; s->h[4] += e;
}

void revo_sha1_hex(const u8 *data, size_t size, char out[41]) {
    Sha1 s = {{0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0}, 0, {0}, 0};
    size_t i = 0;
    for (; i + 64 <= size; i += 64) sha1_block(&s, data + i);
    u8 last[128] = {0};
    size_t rest = size - i;
    memcpy(last, data + i, rest);
    last[rest] = 0x80;
    size_t blocks = rest + 9 > 64 ? 2 : 1;
    u64 bits = (u64)size * 8;
    for (int b = 0; b < 8; b++) last[blocks * 64 - 1 - b] = (u8)(bits >> (8 * b));
    for (size_t b = 0; b < blocks; b++) sha1_block(&s, last + 64 * b);
    for (int k = 0; k < 5; k++) snprintf(out + 8 * k, 9, "%08x", s.h[k]);
}

/* These windows speak the language the game starts in (revo_language): English, French, German, Italian or
   Spanish. */
static const char *tr(const char *const text[REVO_LANG_COUNT]) { return text[revo_language()]; }
#define TR(en, fr, de, it, es) tr((const char *const[REVO_LANG_COUNT]){en, fr, de, it, es})

#ifdef _WIN32
/* The UTF-8 parts as one wide string, each ending in its NUL, and one more NUL after the last (a file dialog's
   filter is such a list). */
static const wchar_t *wide(const char *const *parts, int n, wchar_t *out, int size) {
    int at = 0;
    for (int i = 0; i < n && at < size - 1; i++) {
        const int got = MultiByteToWideChar(CP_UTF8, 0, parts[i], -1, out + at, size - 1 - at);
        if (got <= 0) break;
        at += got;
    }
    out[at] = 0;
    return out;
}
#else
/* s in single quotes, for the shell. */
static void quoted(char *out, size_t size, const char *s) {
    size_t n = 0;
    out[n++] = '\'';
    for (; *s && n + 6 < size; s++) {
        if (*s == '\'') {
            memcpy(out + n, "'\\''", 4);
            n += 4;
        } else {
            out[n++] = *s;
        }
    }
    out[n++] = '\'';
    out[n] = 0;
}
#endif

static const char *pick_title(void) {
    return TR("Pick your Rayman Revolution disc image (.iso) or SLUS_201.38",
              "Choisissez l'image disque (.iso) de Rayman Revolution ou SLUS_201.38",
              "Wählen Sie das Disc-Image (.iso) von Rayman Revolution oder SLUS_201.38",
              "Scegli l'immagine disco (.iso) di Rayman Revolution o SLUS_201.38",
              "Elige la imagen de disco (.iso) de Rayman Revolution o SLUS_201.38");
}

static int ask_choose(const char *text) {
    const SDL_MessageBoxButtonData buttons[] = {
        {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, TR("Choose...", "Choisir...", "Auswählen...", "Scegli...", "Elegir...")},
        {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, TR("Quit", "Quitter", "Beenden", "Esci", "Salir")},
    };
    const SDL_MessageBoxData box = {SDL_MESSAGEBOX_INFORMATION, NULL, "Rayman Re-Revolution", text, 2, buttons, NULL};
    int hit = 0;
    if (SDL_ShowMessageBox(&box, &hit) != 0) {
        fprintf(stderr, "%s\n", text);
        return 0;
    }
    return hit == 1;
}

static void show_error(const char *text) {
    if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Rayman Re-Revolution", text, NULL) != 0)
        fprintf(stderr, "%s\n", text);
}

static int pick_file(char *out, size_t size) {
#ifdef _WIN32
    wchar_t file[MAX_PATH * 4] = L"";
    OPENFILENAMEW ofn;
    memset(&ofn, 0, sizeof ofn);
    ofn.lStructSize = sizeof ofn;
    const char *const filter[6] = {
        TR("Rayman Revolution (disc image, SLUS_201.38)", "Rayman Revolution (image disque, SLUS_201.38)",
           "Rayman Revolution (Disc-Image, SLUS_201.38)", "Rayman Revolution (immagine disco, SLUS_201.38)",
           "Rayman Revolution (imagen de disco, SLUS_201.38)"),
        "*.iso;SLUS_201.38",
        TR("Disc images (*.iso)", "Images disque (*.iso)", "Disc-Images (*.iso)", "Immagini disco (*.iso)",
           "Imágenes de disco (*.iso)"),
        "*.iso",
        TR("All files", "Tous les fichiers", "Alle Dateien", "Tutti i file", "Todos los archivos"),
        "*.*",
    };
    const char *title = pick_title();
    wchar_t wide_filter[512], wide_title[256];
    ofn.lpstrFilter = wide(filter, 6, wide_filter, 512);
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof file / sizeof *file;
    ofn.lpstrTitle = wide(&title, 1, wide_title, 256);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
    if (!GetOpenFileNameW(&ofn)) return 0;
    return WideCharToMultiByte(CP_UTF8, 0, file, -1, out, (int)size, NULL, NULL) > 0;
#else
    char title[600], script[700], cmds[3][1000];
    size_t ncmds = 0;
    quoted(title, sizeof title, pick_title());
#ifdef __APPLE__
    {
        char line[400];
        snprintf(line, sizeof line, "POSIX path of (choose file with prompt \"%s\")", pick_title());
        quoted(script, sizeof script, line);
        snprintf(cmds[ncmds++], sizeof cmds[0], "osascript -e %s 2>/dev/null", script);
    }
#else
    (void)script;
#endif
    snprintf(cmds[ncmds++], sizeof cmds[0], "zenity --file-selection --title=%s 2>/dev/null", title);
    snprintf(cmds[ncmds++], sizeof cmds[0], "kdialog --title %s --getopenfilename . '*.iso SLUS_201.38' 2>/dev/null",
             title);
    for (size_t i = 0; i < ncmds; i++) {
        FILE *p = popen(cmds[i], "r");
        if (!p) continue;
        out[0] = 0;
        char *got = fgets(out, (int)size, p);
        const int code = pclose(p);
        if (got && code == 0) {
            out[strcspn(out, "\r\n")] = 0;
            if (*out) return 1;
        }
        if (code == 1 << 8) return 0;
    }
    return 0;
#endif
}

static void folder_of(char *path) {
    char *slash = strrchr(path, '/'), *back = strrchr(path, '\\');
    if (back > slash) slash = back;
    if (slash) *slash = 0;
    if (strlen(path) == 2 && path[1] == ':') strcat(path, "\\");
#ifdef _WIN32
    wchar_t wide[MAX_PATH * 4], shrt[MAX_PATH * 4];
    char ascii[MAX_PATH * 4];
    if (MultiByteToWideChar(CP_UTF8, 0, path, -1, wide, MAX_PATH * 4) > 0 &&
        GetShortPathNameW(wide, shrt, MAX_PATH * 4) > 0 &&
        WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, shrt, -1, ascii, sizeof ascii, NULL, NULL) > 0) {
        int plain = 1;
        for (const char *c = ascii; *c; c++) plain &= (unsigned char)*c < 128 && *c != '?';
        if (plain) strcpy(path, ascii);
    }
#endif
}

static int make_dirs(const char *path) {
    char p[1100];
    snprintf(p, sizeof p, "%s", path);
    for (char *c = p + 1; ; c++) {
        if (*c == '/' || *c == '\\' || !*c) {
            const char keep = *c;
            *c = 0;
            if (!(strlen(p) == 2 && p[1] == ':')) {
#ifdef _WIN32
                wchar_t w[1100];
                if (MultiByteToWideChar(CP_UTF8, 0, p, -1, w, 1100) > 0) CreateDirectoryW(w, NULL);
#else
                mkdir(p, 0755);
#endif
            }
            *c = keep;
            if (!keep) break;
        }
    }
#ifdef _WIN32
    wchar_t w[1100];
    return MultiByteToWideChar(CP_UTF8, 0, path, -1, w, 1100) > 0 && (GetFileAttributesW(w) & FILE_ATTRIBUTE_DIRECTORY) &&
           GetFileAttributesW(w) != INVALID_FILE_ATTRIBUTES;
#else
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

static void extract_dir(char *out, size_t size, int fallback) {
    if (!fallback) {
#ifdef _WIN32
        wchar_t w[MAX_PATH * 4];
        char exe[MAX_PATH * 4];
        DWORD n = GetModuleFileNameW(NULL, w, MAX_PATH * 4);
        if (n > 0 && n < MAX_PATH * 4 && WideCharToMultiByte(CP_UTF8, 0, w, -1, exe, sizeof exe, NULL, NULL) > 0) {
            char *slash = strrchr(exe, '\\');
            if (slash) *slash = 0;
            snprintf(out, size, "%s\\GAME", exe);
            return;
        }
#endif
    }
    char dir[1024];
    revo_user_dir(dir, sizeof dir);
    snprintf(out, size, "%s/GAME", dir);
}

static void readable_dir(const char *path, char *out, size_t size) {
    snprintf(out, size, "%s", path);
#ifdef _WIN32
    int plain = 1;
    for (const char *c = path; *c; c++) plain &= (unsigned char)*c < 128;
    if (plain) return;
    wchar_t wide[MAX_PATH * 4], shrt[MAX_PATH * 4];
    char ascii[MAX_PATH * 4];
    out[0] = 0;
    if (MultiByteToWideChar(CP_UTF8, 0, path, -1, wide, MAX_PATH * 4) > 0 &&
        GetShortPathNameW(wide, shrt, MAX_PATH * 4) > 0 &&
        WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, shrt, -1, ascii, sizeof ascii, NULL, NULL) > 0) {
        for (const char *c = ascii; *c; c++) if ((unsigned char)*c >= 128 || *c == '?') return;
        snprintf(out, size, "%s", ascii);
    }
#endif
}

#ifdef _WIN32
typedef struct ProgressDialog ProgressDialog;
typedef struct {
    HRESULT(STDMETHODCALLTYPE *QueryInterface)(ProgressDialog *, REFIID, void **);
    ULONG(STDMETHODCALLTYPE *AddRef)(ProgressDialog *);
    ULONG(STDMETHODCALLTYPE *Release)(ProgressDialog *);
    HRESULT(STDMETHODCALLTYPE *StartProgressDialog)(ProgressDialog *, HWND, IUnknown *, DWORD, LPCVOID);
    HRESULT(STDMETHODCALLTYPE *StopProgressDialog)(ProgressDialog *);
    HRESULT(STDMETHODCALLTYPE *SetTitle)(ProgressDialog *, PCWSTR);
    HRESULT(STDMETHODCALLTYPE *SetAnimation)(ProgressDialog *, HINSTANCE, UINT);
    BOOL(STDMETHODCALLTYPE *HasUserCancelled)(ProgressDialog *);
    HRESULT(STDMETHODCALLTYPE *SetProgress)(ProgressDialog *, DWORD, DWORD);
    HRESULT(STDMETHODCALLTYPE *SetProgress64)(ProgressDialog *, ULONGLONG, ULONGLONG);
    HRESULT(STDMETHODCALLTYPE *SetLine)(ProgressDialog *, DWORD, PCWSTR, BOOL, LPCVOID);
    HRESULT(STDMETHODCALLTYPE *SetCancelMsg)(ProgressDialog *, PCWSTR, LPCVOID);
    HRESULT(STDMETHODCALLTYPE *Timer)(ProgressDialog *, DWORD, LPCVOID);
} ProgressDialogVtbl;
struct ProgressDialog { const ProgressDialogVtbl *lpVtbl; };
#endif

typedef struct {
#ifdef _WIN32
    ProgressDialog *dlg;
#endif
    u64 last;
} Progress;

static void progress_start(Progress *p) {
    memset(p, 0, sizeof *p);
#ifdef _WIN32
    static const CLSID clsid = {0xF8383852, 0xFCD3, 0x11d1, {0xA6, 0xB9, 0x00, 0x60, 0x97, 0xDF, 0x5B, 0xD4}};
    static const IID iid = {0xEBBC7C04, 0x315E, 0x11d2, {0xB6, 0x2F, 0x00, 0x60, 0x97, 0xDF, 0x5B, 0xD4}};
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (CoCreateInstance(&clsid, NULL, CLSCTX_INPROC_SERVER, &iid, (void **)&p->dlg) != S_OK) p->dlg = NULL;
    if (p->dlg) {
        p->dlg->lpVtbl->SetTitle(p->dlg, L"Rayman Re-Revolution");
        const char *line = TR("Extracting the game's files (only the first time)...",
                              "Extraction des fichiers du jeu (seulement la première fois)...",
                              "Die Dateien des Spiels werden entpackt (nur beim ersten Start)...",
                              "Estrazione dei file del gioco (solo la prima volta)...",
                              "Extrayendo los archivos del juego (solo la primera vez)...");
        const char *cancel = TR("Cancelled: the game will read its files from the disc instead.",
                                "Annulé : le jeu lira ses fichiers directement sur le disque.",
                                "Abgebrochen: Das Spiel liest seine Dateien stattdessen von der Disc.",
                                "Annullato: il gioco leggerà i suoi file direttamente dal disco.",
                                "Cancelado: el juego leerá sus archivos directamente del disco.");
        wchar_t w[256];
        p->dlg->lpVtbl->SetLine(p->dlg, 1, wide(&line, 1, w, 256), FALSE, NULL);
        p->dlg->lpVtbl->SetCancelMsg(p->dlg, wide(&cancel, 1, w, 256), NULL);
        p->dlg->lpVtbl->StartProgressDialog(p->dlg, NULL, NULL, PROGDLG_NORMAL | PROGDLG_AUTOTIME, NULL);
    }
#endif
}

static int progress_step(Progress *p, const char *name, u64 done, u64 total) {
#ifdef _WIN32
    if (p->dlg) {
        wchar_t w[300];
        if (MultiByteToWideChar(CP_UTF8, 0, name, -1, w, 300) > 0) p->dlg->lpVtbl->SetLine(p->dlg, 2, w, TRUE, NULL);
        p->dlg->lpVtbl->SetProgress64(p->dlg, done, total);
        return !p->dlg->lpVtbl->HasUserCancelled(p->dlg);
    }
#endif
    if (done - p->last >= total / 20 || done == total) {
        revo_log("[game] bigfiles: %llu of %llu MB", (unsigned long long)(done >> 20), (unsigned long long)(total >> 20));
        p->last = done;
    }
    (void)name;
    return 1;
}

static void progress_stop(Progress *p) {
#ifdef _WIN32
    if (p->dlg) {
        p->dlg->lpVtbl->StopProgressDialog(p->dlg);
        p->dlg->lpVtbl->Release(p->dlg);
    }
    CoUninitialize();
#endif
    (void)p;
}

/* Takes a bigfile's files out to dir, as its .DSC lists them; with no Progress, only adds up their
   size. */
static int extract_bigfile(const char *dir, const char *bf, const char *dsc_name, Progress *pr, u64 *done, u64 total,
                           int *cancelled) {
    u32 dsc_size = 0;
    char *dsc = (char *)revo_disc_load(dsc_name, &dsc_size);
    if (!dsc) return 0;
    char stack[16][128];
    int depth = 0;
    u8 *buf = pr ? malloc(1 << 20) : NULL;
    int ok = !pr || buf;
    for (const char *c = dsc; *c && ok;) {
        if (*c == '(') {
            const char *e = c + 1;
            while (*e && !strchr("([)", *e)) e++;
            if (depth < 16) snprintf(stack[depth], sizeof stack[depth], "%.*s", (int)(e - c - 1), c + 1);
            depth++;
            c = e;
        } else if (*c == ')') {
            depth--;
            c++;
            if (depth <= 0) break;
        } else if (*c == '[') {
            const char *e = strchr(c, ']');
            if (!e) break;
            const char *c3 = e, *c2, *c1;
            while (c3 > c && *c3 != ',') c3--;
            c2 = c3 - 1;
            while (c2 > c && *c2 != ',') c2--;
            c1 = c2 - 1;
            while (c1 > c && *c1 != ',') c1--;
            if (c1 > c) {
                const u64 at = strtoull(c1 + 1, NULL, 10);
                const u32 size = (u32)strtoul(c2 + 1, NULL, 10);
                if (!pr) {
                    *done += size;
                } else {
                    char path[1100];
                    int n = snprintf(path, sizeof path, "%s", dir);
                    for (int d = 1; d < depth && d < 16; d++) n += snprintf(path + n, sizeof path - (size_t)n, "/%s", stack[d]);
                    make_dirs(path);
                    snprintf(path + n, sizeof path - (size_t)n, "/%.*s", (int)(c1 - c - 1), c + 1);
                    FILE *out = revo_fopen(path, "wb");
                    ok = out != NULL;
                    for (u32 off = 0; ok && off < size;) {
                        const u32 part = size - off < (1u << 20) ? size - off : (1u << 20);
                        ok = revo_disc_read_file(bf, at + off, part, buf) && fwrite(buf, 1, part, out) == part;
                        off += part;
                        *done += part;
                        if (ok && !progress_step(pr, path + strlen(dir) + 1, *done, total)) {
                            *cancelled = 1;
                            ok = 0;
                        }
                    }
                    if (out && fclose(out) != 0) ok = 0;
                    if (!ok && !*cancelled) revo_log("[game] cannot write %s", path);
                }
            }
            c = e + 1;
        } else {
            c++;
        }
    }
    free(buf);
    free(dsc);
    return ok;
}

#ifdef _WIN32
static u64 free_bytes(const char *dir) {
    wchar_t w[1100];
    ULARGE_INTEGER avail;
    if (MultiByteToWideChar(CP_UTF8, 0, dir, -1, w, 1100) > 0 && GetDiskFreeSpaceExW(w, &avail, NULL, NULL))
        return avail.QuadPart;
    return ~0ull;
}
#endif

#define EXTRACT_FORMAT 1

/* A bigfile to take out: GAME.BF to GAME, and SOUND's, SNDENG.BF to SOUND/SNDENG. */
typedef struct {
    char bf[64], dsc[64], dir[1100];
    u64 size;
} Bigfile;

static int up_to_date(const char *dir, const char *source) {
    char marker[1200], kept[1100] = "", line[64];
    int format = 0;
    snprintf(marker, sizeof marker, "%s/.extracted", dir);
    FILE *m = revo_fopen(marker, "r");
    if (!m) return 0;
    if (fgets(kept, sizeof kept, m)) kept[strcspn(kept, "\r\n")] = 0;
    if (!fgets(line, sizeof line, m) || sscanf(line, "format %d", &format) != 1) format = 1;
    fclose(m);
    if (kept[0] && format != EXTRACT_FORMAT)
        revo_log("[game] %s: format %d, this version unpacks format %d", dir, format, EXTRACT_FORMAT);
    return !strcmp(kept, source) && format == EXTRACT_FORMAT;
}

static void mark(const char *dir, const char *source) {
    char marker[1200];
    snprintf(marker, sizeof marker, "%s/.extracted", dir);
    FILE *m = revo_fopen(marker, "w");
    if (!m) return;
    fprintf(m, "%s\nformat %d\n", source, EXTRACT_FORMAT);
    fclose(m);
}

static void copy_disc_files(const char *root, const char *source) {
    char disc[1200], marker[1300];
    snprintf(disc, sizeof disc, "%s/DISC", root);
    if (up_to_date(disc, source)) return;
    snprintf(marker, sizeof marker, "%s/.extracted", disc);
    remove(marker);
    u32 size;
    const char *name;
    for (int i = 0; (name = revo_disc_file(i, &size)); i++) {
        const size_t len = strlen(name);
        if (len > 3 && !strcmp(name + len - 3, ".BF")) continue;
        char path[1300];
        snprintf(path, sizeof path, "%s/%s", disc, name);
        char *last = NULL;
        for (char *c = path + strlen(disc) + 1; *c; c++)
            if (*c == '\\') *(last = c) = '/';
        if (last) {
            *last = 0;
            const int made = make_dirs(path);
            *last = '/';
            if (!made) return;
        } else if (!make_dirs(disc)) {
            return;
        }
        u32 n = 0;
        u8 *data = revo_disc_load(name, &n);
        FILE *out = data ? revo_fopen(path, "wb") : NULL;
        const int ok = out && fwrite(data, 1, n, out) == n;
        if (out && fclose(out) != 0) {
            free(data);
            revo_log("[game] cannot write %s", path);
            return;
        }
        free(data);
        if (!ok) {
            revo_log("[game] cannot write %s", path);
            return;
        }
    }
    mark(disc, source);
    revo_log("[game] the disc's other files copied to %s", disc);
}

static void use_extracted(const char *source) {
    for (int fallback = 0; fallback < 2; fallback++) {
        char dir[1100], root[1100], sound[1200], readable[1100];
        extract_dir(dir, sizeof dir, fallback);
        snprintf(root, sizeof root, "%.*s", (int)(strlen(dir) - 5), dir);
        snprintf(sound, sizeof sound, "%s/SOUND", root);
        static Bigfile todo[32];
        int n = 0;
        int game = 1;
        if (!up_to_date(dir, source)) {
            Bigfile *b = &todo[n++];
            snprintf(b->bf, sizeof b->bf, "GAME.BF");
            snprintf(b->dsc, sizeof b->dsc, "GAME.DSC");
            snprintf(b->dir, sizeof b->dir, "%s", dir);
        }
        u32 size;
        const char *name;
        for (int i = 0; (name = revo_disc_file(i, &size)) && n < 32; i++) {
            const size_t len = strlen(name);
            if (strncmp(name, "SOUND\\", 6) || len < 10 || strcmp(name + len - 3, ".BF")) continue;
            Bigfile *b = &todo[n];
            snprintf(b->bf, sizeof b->bf, "%s", name);
            snprintf(b->dsc, sizeof b->dsc, "%.*s.DSC", (int)(len - 3), name);
            snprintf(b->dir, sizeof b->dir, "%s/%.*s", sound, (int)(len - 9), name + 6);
            if (!up_to_date(b->dir, source)) n++;
        }
        u64 total = 0;
        for (int i = 0; i < n; i++) {
            todo[i].size = 0;
            int unused = 0;
            extract_bigfile(todo[i].dir, todo[i].bf, todo[i].dsc, NULL, &todo[i].size, 0, &unused);
            total += todo[i].size;
        }
        if (n && !make_dirs(dir)) continue;
#ifdef _WIN32
        /* Room for all of it, or GAME.BF alone, or nothing. */
        const u64 room = free_bytes(dir);
        if (n && room < total + (300ull << 20)) {
            const int has_game = !strcmp(todo[0].bf, "GAME.BF");
            revo_log("[game] %s: not enough room for SOUND's bigfiles, read from the disc", root);
            n = has_game;
            total = has_game ? todo[0].size : 0;
            if (has_game && room < (1800ull << 20)) {
                revo_log("[game] %s: not enough room for GAME.BF, read from the disc", dir);
                continue;
            }
        }
#endif
        if (n) {
            Progress pr;
            progress_start(&pr);
            u64 done = 0;
            int cancelled = 0;
            for (int i = 0; i < n && !cancelled; i++) {
                const int is_game = !strcmp(todo[i].bf, "GAME.BF");
                char marker[1200];
                snprintf(marker, sizeof marker, "%s/.extracted", todo[i].dir);
                remove(marker);
                const int ok = make_dirs(todo[i].dir) &&
                               extract_bigfile(todo[i].dir, todo[i].bf, todo[i].dsc, &pr, &done, total, &cancelled);
                if (ok) {
                    mark(todo[i].dir, source);
                    revo_log("[game] %s extracted to %s", todo[i].bf, todo[i].dir);
                } else if (is_game) {
                    game = 0;
                }
                if (cancelled) revo_log("[game] %s: cancelled, read from the disc", todo[i].bf);
            }
            progress_stop(&pr);
            if (cancelled) return;
            if (!game) continue;
        }
        copy_disc_files(root, source);
        readable_dir(dir, readable, sizeof readable);
        if (!readable[0]) continue;
        snprintf(revo_bf_dir_default, sizeof revo_bf_dir_default, "%s", readable);
        readable_dir(sound, readable, sizeof readable);
        snprintf(revo_sound_dir_default, sizeof revo_sound_dir_default, "%s", readable);
        revo_disc_open(source);
        return;
    }
}

static int check(const char *source, char *why, size_t size);

/* The game's files taken out of the disc: GAME and SOUND's bigfiles, and DISC with the rest of the disc. */
static int open_installed(const char *source) {
    for (int fallback = 0; fallback < 2; fallback++) {
        char dir[1100], root[1100], sound[1200], disc[1200], readable[1100], why[1400];
        extract_dir(dir, sizeof dir, fallback);
        snprintf(root, sizeof root, "%.*s", (int)(strlen(dir) - 5), dir);
        snprintf(sound, sizeof sound, "%s/SOUND", root);
        snprintf(disc, sizeof disc, "%s/DISC", root);
        if (!up_to_date(dir, source) || !up_to_date(disc, source)) continue;
        readable_dir(dir, readable, sizeof readable);
        if (!readable[0]) continue;
        snprintf(revo_bf_dir_default, sizeof revo_bf_dir_default, "%s", readable);
        readable_dir(sound, readable, sizeof readable);
        snprintf(revo_sound_dir_default, sizeof revo_sound_dir_default, "%s", readable);
        readable_dir(disc, readable, sizeof readable);
        int complete = readable[0] && check(readable, why, sizeof why);
        u32 size;
        const char *name;
        for (int i = 0; complete && (name = revo_disc_file(i, &size)); i++) {
            const size_t len = strlen(name);
            if (strncmp(name, "SOUND\\", 6) || len < 11 || strcmp(name + len - 4, ".DSC")) continue;
            char folder[1300];
            snprintf(folder, sizeof folder, "%s/%.*s", sound, (int)(len - 10), name + 6);
            complete = up_to_date(folder, source);
        }
        if (complete) {
            revo_log("[game] %s is not there: the game's files in %s are used", source, root);
            return 1;
        }
        revo_bf_dir_default[0] = revo_sound_dir_default[0] = 0;
    }
    return 0;
}

static void kept_path(char *out, size_t size) {
    char dir[1024];
    revo_user_dir(dir, sizeof dir);
    snprintf(out, size, "%s/game.txt", dir);
}

static int load_kept(char *out, size_t size) {
    char file[1100];
    kept_path(file, sizeof file);
    FILE *f = revo_fopen(file, "r");
    if (!f) return 0;
    char *got = fgets(out, (int)size, f);
    fclose(f);
    if (!got) return 0;
    out[strcspn(out, "\r\n")] = 0;
    return *out != 0;
}

static void keep(const char *source) {
    char file[1100];
    kept_path(file, sizeof file);
    FILE *f = revo_fopen(file, "w");
    if (!f) { revo_log("[game] cannot write %s", file); return; }
    fprintf(f, "%s\n", source);
    fclose(f);
}

static int check(const char *source, char *why, size_t size) {
    if (!revo_disc_open(source)) {
        snprintf(why, size, TR("Couldn't find SLUS_201.38 in\n%s\n\nYou need the US PS2 disc of Rayman Revolution "
                               "(SLUS-20138), as an .iso image or as the disc's files.",
                               "SLUS_201.38 est introuvable dans\n%s\n\nIl faut le disque PS2 américain de Rayman "
                               "Revolution (SLUS-20138), en image .iso ou avec les fichiers du disque.",
                               "SLUS_201.38 wurde nicht gefunden in\n%s\n\nBenötigt wird die US-PS2-Disc von Rayman "
                               "Revolution (SLUS-20138), als .iso-Image oder als Dateien der Disc.",
                               "SLUS_201.38 non trovato in\n%s\n\nServe il disco PS2 americano di Rayman Revolution "
                               "(SLUS-20138), come immagine .iso o con i file del disco.",
                               "No se encontró SLUS_201.38 en\n%s\n\nHace falta el disco de PS2 estadounidense de "
                               "Rayman Revolution (SLUS-20138), como imagen .iso o con los archivos del disco."), source);
        return 0;
    }
    u32 n = 0;
    u8 *elf = revo_full_program() ? NULL : revo_disc_load("SLUS_201.38", &n);
    char sha1[41] = "";
    if (elf) revo_sha1_hex(elf, n, sha1);
    free(elf);
    if (!revo_full_program() && strcmp(sha1, RETAIL_SHA1)) {
        snprintf(why, size, TR("This isn't the right version of the game (the SHA-1 of SLUS_201.38 is %s).\n\n"
                               "Only the US version of Rayman Revolution (SLUS-20138) works, PAL and Japanese discs "
                               "won't.",
                               "Ce n'est pas la bonne version du jeu (le SHA-1 de SLUS_201.38 est %s).\n\n"
                               "Seule la version américaine de Rayman Revolution (SLUS-20138) fonctionne, pas les "
                               "disques PAL ni japonais.",
                               "Das ist nicht die richtige Version des Spiels (der SHA-1 von SLUS_201.38 ist %s).\n\n"
                               "Nur die US-Version von Rayman Revolution (SLUS-20138) funktioniert, PAL- und "
                               "japanische Discs nicht.",
                               "Questa non è la versione giusta del gioco (lo SHA-1 di SLUS_201.38 è %s).\n\n"
                               "Funziona solo la versione americana di Rayman Revolution (SLUS-20138), non i dischi PAL "
                               "né quelli giapponesi.",
                               "Esta no es la versión correcta del juego (el SHA-1 de SLUS_201.38 es %s).\n\n"
                               "Solo funciona la versión estadounidense de Rayman Revolution (SLUS-20138), no los discos "
                               "PAL ni japoneses."), sha1);
        return 0;
    }
    u8 *dsc = revo_disc_load("GAME.DSC", NULL);
    if (!dsc) {
        snprintf(why, size, TR("Some of the game's files are missing: GAME.DSC isn't in\n%s",
                               "Il manque des fichiers du jeu : GAME.DSC n'est pas dans\n%s",
                               "Es fehlen Dateien des Spiels: GAME.DSC ist nicht in\n%s",
                               "Mancano dei file del gioco: GAME.DSC non è in\n%s",
                               "Faltan archivos del juego: GAME.DSC no está en\n%s"), source);
        return 0;
    }
    free(dsc);
    return 1;
}

int revo_game_open(int argc, char **argv, const char *dev_default) {
    const int again = argc > 1 && !strcmp(argv[1], "--choose-game");
    const char *given = again ? NULL : argc > 1 ? argv[1] : getenv("REVO_ASSETS");
    if (given && *given) {
        if (revo_disc_open(given)) return 1;
        fprintf(stderr, "no SLUS_201.38 in %s\n", given);
        return 0;
    }
    char source[1100], why[1400];
    if (again) {
    } else if (load_kept(source, sizeof source)) {
        if (check(source, why, sizeof why)) {
            use_extracted(source);
            return 1;
        }
        if (open_installed(source)) return 1;
        revo_log("[game] %s: no longer the game", source);
    } else if (dev_default && *dev_default && revo_disc_open(dev_default)) {
        return 1;
    }
    const char *text = TR("Rayman Re-Revolution needs your own copy of the game:\n"
                          "the US PS2 disc of Rayman Revolution (SLUS-20138).\n\n"
                          "Pick its disc image (.iso), or the file SLUS_201.38\n"
                          "on the disc or in a folder with the disc's files.\n\n"
                          "You'll only be asked once.",
                          "Rayman Re-Revolution a besoin de votre propre exemplaire du jeu :\n"
                          "le disque PS2 américain de Rayman Revolution (SLUS-20138).\n\n"
                          "Choisissez son image disque (.iso), ou le fichier SLUS_201.38\n"
                          "sur le disque ou dans un dossier avec les fichiers du disque.\n\n"
                          "Ce n'est demandé qu'une fois.",
                          "Rayman Re-Revolution benötigt Ihr eigenes Exemplar des Spiels:\n"
                          "die US-PS2-Disc von Rayman Revolution (SLUS-20138).\n\n"
                          "Wählen Sie ihr Disc-Image (.iso) oder die Datei SLUS_201.38\n"
                          "auf der Disc oder in einem Ordner mit den Dateien der Disc.\n\n"
                          "Das wird nur einmal abgefragt.",
                          "Rayman Re-Revolution ha bisogno della tua copia del gioco:\n"
                          "il disco PS2 americano di Rayman Revolution (SLUS-20138).\n\n"
                          "Scegli la sua immagine disco (.iso) o il file SLUS_201.38\n"
                          "sul disco o in una cartella con i file del disco.\n\n"
                          "Te lo chiederà solo una volta.",
                          "Rayman Re-Revolution necesita tu propia copia del juego:\n"
                          "el disco de PS2 estadounidense de Rayman Revolution (SLUS-20138).\n\n"
                          "Elige su imagen de disco (.iso) o el archivo SLUS_201.38\n"
                          "en el disco o en una carpeta con los archivos del disco.\n\n"
                          "Solo se te pedirá una vez.");
    while (ask_choose(text)) {
        if (!pick_file(source, sizeof source)) continue;
        const char *base = strrchr(source, '/');
        const char *back = strrchr(source, '\\');
        if (back > base) base = back;
        base = base ? base + 1 : source;
        if (!SDL_strcasecmp(base, "SLUS_201.38") || !SDL_strncasecmp(base, "SLUS_201.38;", 12)) folder_of(source);
        if (check(source, why, sizeof why)) {
            keep(source);
            revo_log("[game] %s", source);
            use_extracted(source);
            return 1;
        }
        show_error(why);
    }
    return 0;
}

static void choose_again_at_exit(void) {
#ifdef _WIN32
    wchar_t exe[MAX_PATH], cmd[MAX_PATH + 32];
    const DWORD n = GetModuleFileNameW(NULL, exe, MAX_PATH);
    if (!n || n >= MAX_PATH) return;
    swprintf(cmd, sizeof cmd / sizeof *cmd, L"\"%ls\" --choose-game", exe);
    STARTUPINFOW si = {sizeof si};
    PROCESS_INFORMATION pi;
    if (!CreateProcessW(exe, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return;
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
#else
    char exe[4096];
#ifdef __APPLE__
    uint32_t size = sizeof exe;
    if (_NSGetExecutablePath(exe, &size)) return;
#else
    const ssize_t n = readlink("/proc/self/exe", exe, sizeof exe - 1);
    if (n <= 0) return;
    exe[n] = 0;
#endif
    unsetenv("REVO_NATIVE_NO_ASLR");
    fflush(NULL);
    char *argv[] = {exe, "--choose-game", NULL};
    execv(exe, argv);
#endif
}

void revo_game_choose_again(void) {
    static int armed;
    if (!armed) atexit(choose_again_at_exit);
    armed = 1;
    SDL_Event quit = {0};
    quit.type = SDL_QUIT;
    SDL_PushEvent(&quit);
}
