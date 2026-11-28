#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#define COBJMACROS
#include <direct.h>
#include <windows.h>
#include <initguid.h>
#include <knownfolders.h>
#include <shlobj.h>
#include <shobjidl.h>
#endif

#include "runtime.h"

static int is_folder(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && (st.st_mode & S_IFDIR) != 0;
}

static void make_folder(const char *path) {
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

static const char *home(void) {
    const char *h = getenv("HOME");
#ifdef _WIN32
    if (!h || !*h) h = getenv("USERPROFILE");
#endif
    return h && *h ? h : ".";
}

static void c_path(char *path, size_t size) {
#ifdef _WIN32
    int plain = 1;
    for (const char *c = path; *c; c++) plain &= (unsigned char)*c < 128;
    if (plain) return;
    wchar_t wide[MAX_PATH * 4], shrt[MAX_PATH * 4];
    char ascii[MAX_PATH * 4];
    if (MultiByteToWideChar(CP_UTF8, 0, path, -1, wide, MAX_PATH * 4) <= 0) return;
    CreateDirectoryW(wide, NULL);
    if (GetShortPathNameW(wide, shrt, MAX_PATH * 4) <= 0 ||
        WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, shrt, -1, ascii, sizeof ascii, NULL, NULL) <= 0)
        return;
    for (const char *c = ascii; *c; c++) if ((unsigned char)*c >= 128 || *c == '?') return;
    snprintf(path, size, "%s", ascii);
#else
    (void)path;
    (void)size;
#endif
}

static void standard_folder(char *out, size_t size) {
#ifdef _WIN32
    PWSTR saved = NULL;
    if (SHGetKnownFolderPath(&FOLDERID_SavedGames, KF_FLAG_CREATE, NULL, &saved) == S_OK) {
        char base[MAX_PATH * 3];
        const int n = WideCharToMultiByte(CP_UTF8, 0, saved, -1, base, (int)sizeof base, NULL, NULL);
        CoTaskMemFree(saved);
        if (n > 0) {
            snprintf(out, size, "%s\\Rayman Revolution", base);
            c_path(out, size);
            return;
        }
    }
    snprintf(out, size, "%s/Saved Games/Rayman Revolution", home());
#else
    const char *data = getenv("XDG_DATA_HOME");
    if (data && *data) {
        snprintf(out, size, "%s/rayman-revolution", data);
    } else {
        char share[1024];
        snprintf(share, sizeof share, "%s/.local", home());
        make_folder(share);
        snprintf(share, sizeof share, "%s/.local/share", home());
        make_folder(share);
        snprintf(out, size, "%s/rayman-revolution", share);
    }
#endif
}

static void program_folder(char *out, size_t size) {
    out[0] = 0;
#ifdef _WIN32
    wchar_t w[MAX_PATH * 4];
    char exe[MAX_PATH * 4];
    const DWORD n = GetModuleFileNameW(NULL, w, MAX_PATH * 4);
    if (n == 0 || n >= MAX_PATH * 4 || WideCharToMultiByte(CP_UTF8, 0, w, -1, exe, sizeof exe, NULL, NULL) <= 0) return;
    char *slash = strrchr(exe, '\\');
    if (slash) *slash = 0;
    snprintf(out, size, "%s", exe);
#endif
}

static int read_line(const char *file, char *out, size_t size) {
    FILE *f = revo_fopen(file, "r");
    if (!f) return 0;
    char *got = fgets(out, (int)size, f);
    fclose(f);
    if (!got) return 0;
    out[strcspn(out, "\r\n")] = 0;
    return *out != 0;
}

static int write_line(const char *file, const char *text) {
    FILE *f = revo_fopen(file, "w");
    if (!f) return 0;
    fprintf(f, "%s\n", text);
    return fclose(f) == 0;
}

static int has_saves(const char *dir) {
    char p[1100];
    snprintf(p, sizeof p, "%s/settings.ini", dir);
    FILE *f = revo_fopen(p, "r");
    if (f) { fclose(f); return 1; }
    snprintf(p, sizeof p, "%s/memcard0", dir);
    return is_folder(p);
}

static int copy_file(const char *from, const char *to) {
    FILE *in = revo_fopen(from, "rb");
    if (!in) return 0;
    FILE *out = revo_fopen(to, "wb");
    int ok = out != NULL;
    char buf[1 << 16];
    for (size_t n; ok && (n = fread(buf, 1, sizeof buf, in)) > 0;) ok = fwrite(buf, 1, n, out) == n;
    fclose(in);
    if (out && fclose(out) != 0) ok = 0;
    return ok;
}

static int copy_tree(const char *from, const char *to) {
    make_folder(to);
    DIR *d = opendir(from);
    if (!d) return 0;
    int ok = 1;
    for (struct dirent *e; (e = readdir(d));) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
        char a[1100], b[1100];
        snprintf(a, sizeof a, "%s/%s", from, e->d_name);
        snprintf(b, sizeof b, "%s/%s", to, e->d_name);
        ok &= is_folder(a) ? copy_tree(a, b) : copy_file(a, b);
    }
    closedir(d);
    return ok;
}

static void copy_player_files(const char *from, const char *to) {
    static const char *const files[] = {"settings.ini", "game.txt"};
    static const char *const trees[] = {"memcard0", "screenshots"};
    int ok = 1;
    for (size_t i = 0; i < sizeof files / sizeof *files; i++) {
        char a[1100], b[1100];
        snprintf(a, sizeof a, "%s/%s", from, files[i]);
        snprintf(b, sizeof b, "%s/%s", to, files[i]);
        FILE *f = revo_fopen(a, "rb");
        if (!f) continue;
        fclose(f);
        ok &= copy_file(a, b);
    }
    for (size_t i = 0; i < sizeof trees / sizeof *trees; i++) {
        char a[1100], b[1100];
        snprintf(a, sizeof a, "%s/%s", from, trees[i]);
        snprintf(b, sizeof b, "%s/%s", to, trees[i]);
        if (is_folder(a)) ok &= copy_tree(a, b);
    }
    revo_log("[user] the player's files copied from %s%s", from, ok ? "" : " (not all)");
}

static char g_standard[1024], g_dir[1024];

static void resolve(void) {
    char legacy[1024], prog[1024], marker[1100], chosen[1100];
    standard_folder(g_standard, sizeof g_standard);
    snprintf(legacy, sizeof legacy, "%s/.revolution-native", home());
    if (!is_folder(g_standard) && is_folder(legacy)) snprintf(g_standard, sizeof g_standard, "%s", legacy);
    else make_folder(g_standard);
    snprintf(g_dir, sizeof g_dir, "%s", g_standard);

    program_folder(prog, sizeof prog);
    snprintf(marker, sizeof marker, "%s/portable.txt", prog);
    FILE *portable = prog[0] ? revo_fopen(marker, "r") : NULL;
    if (portable) {
        fclose(portable);
        snprintf(chosen, sizeof chosen, "%s/User", prog);
    } else {
        snprintf(marker, sizeof marker, "%s/location.txt", g_standard);
        if (!read_line(marker, chosen, sizeof chosen)) return;
    }
    c_path(chosen, sizeof chosen);
    make_folder(chosen);
    if (!is_folder(chosen)) {
        revo_log("[user] %s cannot be made: the standard folder is used", chosen);
        return;
    }
    snprintf(g_dir, sizeof g_dir, "%s", chosen);
    char note[1100], from[1100];
    snprintf(note, sizeof note, "%s/.copy_from", g_dir);
    if (read_line(note, from, sizeof from)) {
        if (!has_saves(g_dir) && strcmp(from, g_dir)) copy_player_files(from, g_dir);
        remove(note);
    }
}

void revo_user_dir(char *out, size_t size) {
    if (!g_dir[0]) {
        resolve();
        revo_log_open(g_dir);
        revo_log("[user] the player's folder: %s", g_dir);
    }
    snprintf(out, size, "%s", g_dir);
}

int revo_user_dir_pick(char *out, size_t size) {
#ifdef _WIN32
    static const CLSID clsid = {0xDC1C5A9C, 0xE88A, 0x4DDE, {0xA5, 0xA1, 0x60, 0xF8, 0x2A, 0x20, 0xAE, 0xF7}};
    static const IID iid = {0xD57C7288, 0xD4AD, 0x4768, {0xBE, 0x02, 0x9D, 0x96, 0x95, 0x32, 0xD9, 0x60}};
    const HRESULT init = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    IFileOpenDialog *dlg = NULL;
    int ok = 0;
    if (CoCreateInstance(&clsid, NULL, CLSCTX_INPROC_SERVER, &iid, (void **)&dlg) == S_OK) {
        DWORD opts = 0;
        IFileOpenDialog_GetOptions(dlg, &opts);
        IFileOpenDialog_SetOptions(dlg, opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
        IFileOpenDialog_SetTitle(dlg, L"Rayman Re-Revolution: where to keep the saves and settings");
        IShellItem *item = NULL;
        PWSTR path = NULL;
        if (IFileOpenDialog_Show(dlg, NULL) == S_OK && IFileOpenDialog_GetResult(dlg, &item) == S_OK &&
            IShellItem_GetDisplayName(item, SIGDN_FILESYSPATH, &path) == S_OK) {
            ok = WideCharToMultiByte(CP_UTF8, 0, path, -1, out, (int)size, NULL, NULL) > 0;
            CoTaskMemFree(path);
        }
        if (item) IShellItem_Release(item);
        IFileOpenDialog_Release(dlg);
    }
    if (SUCCEEDED(init)) CoUninitialize();
    return ok;
#else
    (void)out;
    (void)size;
    return 0;
#endif
}

int revo_user_dir_set(const char *to, int *had_saves) {
    char prog[1024], portable[1100], location[1100], target[1100];
    if (!g_dir[0]) revo_user_dir(target, sizeof target);
    program_folder(prog, sizeof prog);
    snprintf(portable, sizeof portable, "%s/portable.txt", prog);
    snprintf(location, sizeof location, "%s/location.txt", g_standard);
    if (!to) {
        if (!prog[0]) return 0;
        snprintf(target, sizeof target, "%s/User", prog);
    } else {
        snprintf(target, sizeof target, "%s", *to ? to : g_standard);
    }
    char usable[1100];
    snprintf(usable, sizeof usable, "%s", target);
    c_path(usable, sizeof usable);
    make_folder(usable);
    if (!is_folder(usable)) return 0;
    if (!to) {
        if (!write_line(portable, "Rayman Re-Revolution keeps its saves and settings in the User folder next to it.")) return 0;
    } else {
        remove(portable);
        if (*to && strcmp(to, g_standard)) write_line(location, to);
        else remove(location);
    }
    *had_saves = has_saves(usable);
    if (!*had_saves && strcmp(usable, g_dir)) {
        char note[1200];
        snprintf(note, sizeof note, "%s/.copy_from", usable);
        write_line(note, g_dir);
    }
    revo_log("[user] the player's folder from the next start: %s", target);
    return 1;
}
