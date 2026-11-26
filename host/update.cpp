#include <SDL.h>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include "imgui.h"
#include "mods.h"
#include "ui.h"
#include "update.h"

extern "C" {
#include "runtime.h"
}

#if __has_include("revo_version.h")
#include "revo_version.h"
#endif
#ifndef REVO_VERSION
#define REVO_VERSION "dev"
#endif

#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#include <bcrypt.h>
#include <cctype>
#ifndef WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY
#define WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY 4
#endif
#endif

#define REVO_UPDATE_REPO "got4n/rayman2-revolution-pc"

static const char k_version_tag[] = "REVO_VERSION=" REVO_VERSION;

extern "C" const char *revo_version(void) { return k_version_tag + sizeof "REVO_VERSION=" - 1; }

namespace {

const char *const k_releases_page = "https://github.com/" REVO_UPDATE_REPO "/releases";

enum State { S_IDLE, S_CHECKING, S_CURRENT, S_AVAILABLE, S_FAILED, S_DOWNLOADING, S_INSTALLING, S_INSTALLED,
             S_INSTALL_FAILED };

struct Release { std::string tag, version, notes, page, zip; };

std::mutex g_lock;
State g_state = S_IDLE;
Release g_rel;
std::string g_error;
std::atomic<bool> g_busy{false}, g_cancel{false}, g_restart{false};
std::atomic<unsigned long long> g_done{0}, g_total{0};
std::atomic<Uint32> g_badge_since{0};
bool g_at_start;

void set_state(State s, const std::string &error = std::string()) {
    std::lock_guard<std::mutex> l(g_lock);
    g_state = s;
    g_error = error;
}

bool parse_version(const std::string &s, int out[3]) {
    const char *p = s.c_str();
    if (*p == 'v' || *p == 'V') p++;
    out[0] = out[1] = out[2] = 0;
    return *p >= '0' && *p <= '9' && sscanf(p, "%d.%d.%d", &out[0], &out[1], &out[2]) >= 2;
}

bool newer(const std::string &than_this, const std::string &candidate) {
    int a[3], b[3];
    if (!parse_version(candidate, b)) return false;
    if (!parse_version(than_this, a)) return true;
    return std::lexicographical_compare(a, a + 3, b, b + 3);
}

struct Json {
    const char *p, *end;
    void ws() { while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++; }
    bool at(char c) { ws(); return p < end && *p == c; }
};

void put_utf8(std::string *out, unsigned cp) {
    if (cp < 0x80) out->push_back((char)cp);
    else if (cp < 0x800) { out->push_back((char)(0xC0 | cp >> 6)); out->push_back((char)(0x80 | (cp & 63))); }
    else if (cp < 0x10000) {
        out->push_back((char)(0xE0 | cp >> 12));
        out->push_back((char)(0x80 | (cp >> 6 & 63)));
        out->push_back((char)(0x80 | (cp & 63)));
    } else {
        out->push_back((char)(0xF0 | cp >> 18));
        out->push_back((char)(0x80 | (cp >> 12 & 63)));
        out->push_back((char)(0x80 | (cp >> 6 & 63)));
        out->push_back((char)(0x80 | (cp & 63)));
    }
}

bool hex4(Json &j, unsigned *v) {
    if (j.end - j.p < 4) return false;
    *v = 0;
    for (int i = 0; i < 4; i++) {
        const char c = *j.p++;
        const int d = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
        if (d < 0) return false;
        *v = *v << 4 | (unsigned)d;
    }
    return true;
}

bool json_string(Json &j, std::string *out) {
    if (!j.at('"')) return false;
    j.p++;
    while (j.p < j.end && *j.p != '"') {
        char c = *j.p++;
        if (c == '\\') {
            if (j.p >= j.end) return false;
            const char e = *j.p++;
            switch (e) {
            case 'n': c = '\n'; break;
            case 't': c = '\t'; break;
            case 'r': c = '\r'; break;
            case 'b': c = '\b'; break;
            case 'f': c = '\f'; break;
            case 'u': {
                unsigned cp;
                if (!hex4(j, &cp)) return false;
                if (cp >= 0xD800 && cp < 0xDC00 && j.end - j.p >= 6 && j.p[0] == '\\' && j.p[1] == 'u') {
                    j.p += 2;
                    unsigned lo;
                    if (!hex4(j, &lo)) return false;
                    cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                }
                if (out) put_utf8(out, cp);
                continue;
            }
            default: c = e; break;
            }
        }
        if (out) out->push_back(c);
    }
    if (j.p >= j.end) return false;
    j.p++;
    return true;
}

bool json_skip(Json &j, int depth = 0);

bool json_object(Json &j, const std::function<bool(const std::string &, Json &)> &member) {
    if (!j.at('{')) return false;
    j.p++;
    if (j.at('}')) { j.p++; return true; }
    for (;;) {
        std::string key;
        if (!json_string(j, &key) || !j.at(':')) return false;
        j.p++;
        j.ws();
        if (!member(key, j)) return false;
        if (j.at(',')) { j.p++; continue; }
        if (j.at('}')) { j.p++; return true; }
        return false;
    }
}

bool json_array(Json &j, const std::function<bool(Json &)> &item) {
    if (!j.at('[')) return false;
    j.p++;
    if (j.at(']')) { j.p++; return true; }
    for (;;) {
        j.ws();
        if (!item(j)) return false;
        if (j.at(',')) { j.p++; continue; }
        if (j.at(']')) { j.p++; return true; }
        return false;
    }
}

bool json_skip(Json &j, int depth) {
    if (depth > 64) return false;
    j.ws();
    if (j.p >= j.end) return false;
    if (*j.p == '"') return json_string(j, nullptr);
    if (*j.p == '{') return json_object(j, [depth](const std::string &, Json &v) { return json_skip(v, depth + 1); });
    if (*j.p == '[') return json_array(j, [depth](Json &v) { return json_skip(v, depth + 1); });
    const char *start = j.p;
    while (j.p < j.end && *j.p != ',' && *j.p != '}' && *j.p != ']' && *j.p != ' ' && *j.p != '\r' && *j.p != '\n' &&
           *j.p != '\t')
        j.p++;
    return j.p > start;
}

bool json_text(Json &j, std::string *out) {
    if (j.at('"')) return json_string(j, out);
    return json_skip(j);
}

bool ends_with(const std::string &s, const char *tail) {
    const size_t n = strlen(tail);
    return s.size() >= n && !s.compare(s.size() - n, n, tail);
}

bool parse_release(const std::string &text, Release *r) {
    Json j{text.data(), text.data() + text.size()};
    return json_object(j, [r](const std::string &k, Json &v) {
        if (k == "tag_name") return json_text(v, &r->tag);
        if (k == "html_url") return json_text(v, &r->page);
        if (k == "body") return json_text(v, &r->notes);
        if (k == "assets")
            return json_array(v, [r](Json &a) {
                std::string name, url;
                const bool ok = json_object(a, [&](const std::string &ak, Json &av) {
                    if (ak == "name") return json_text(av, &name);
                    if (ak == "browser_download_url") return json_text(av, &url);
                    return json_skip(av);
                });
                if (ok && ends_with(name, "-windows-x64.zip")) r->zip = url;
                return ok;
            });
        return json_skip(v);
    });
}

using Sink = std::function<bool(const char *, size_t)>;

#ifdef _WIN32
std::wstring wide(const std::string &s) {
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(n > 1 ? n - 1 : 0, L'\0');
    if (n > 1) MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], n);
    return w;
}

std::string narrow(const std::wstring &w) {
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(n > 1 ? n - 1 : 0, '\0');
    if (n > 1) WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], n, nullptr, nullptr);
    return s;
}

std::string windows_error(const char *what) {
    char t[160];
    snprintf(t, sizeof t, "%s (error %lu)", what, (unsigned long)GetLastError());
    return t;
}

bool http_get(const std::string &url, const char *accept, const Sink &sink, std::string *error) {
    const std::wstring w = wide(url);
    wchar_t host[256], path[4096], extra[4096];
    URL_COMPONENTS u = {};
    u.dwStructSize = sizeof u;
    u.lpszHostName = host;
    u.dwHostNameLength = 256;
    u.lpszUrlPath = path;
    u.dwUrlPathLength = 4096;
    u.lpszExtraInfo = extra;
    u.dwExtraInfoLength = 4096;
    if (!WinHttpCrackUrl(w.c_str(), 0, 0, &u)) {
        *error = "bad address: " + url;
        return false;
    }
    const std::wstring host_name(host, u.dwHostNameLength);
    const std::wstring object = std::wstring(path, u.dwUrlPathLength) + std::wstring(extra, u.dwExtraInfoLength);
    HINTERNET s = WinHttpOpen(L"RaymanRevolution-PC", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME,
                              WINHTTP_NO_PROXY_BYPASS, 0);
    if (!s) s = WinHttpOpen(L"RaymanRevolution-PC", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
                            WINHTTP_NO_PROXY_BYPASS, 0);
    if (!s) {
        *error = windows_error("no connection");
        return false;
    }
    WinHttpSetTimeouts(s, 15000, 15000, 15000, 30000);
    HINTERNET c = WinHttpConnect(s, host_name.c_str(), u.nPort, 0);
    HINTERNET r = c ? WinHttpOpenRequest(c, L"GET", object.c_str(), nullptr, WINHTTP_NO_REFERER,
                                         WINHTTP_DEFAULT_ACCEPT_TYPES,
                                         u.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0)
                    : nullptr;
    bool ok = false;
    const std::wstring headers = L"Accept: " + wide(accept) + L"\r\n";
    if (!r) {
        *error = windows_error(("no connection to " + narrow(host_name)).c_str());
    } else if (!WinHttpSendRequest(r, headers.c_str(), (DWORD)-1L, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
               !WinHttpReceiveResponse(r, nullptr)) {
        *error = windows_error(("no answer from " + narrow(host_name)).c_str());
    } else {
        DWORD code = 0, size = sizeof code;
        WinHttpQueryHeaders(r, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX,
                            &code, &size, WINHTTP_NO_HEADER_INDEX);
        DWORD length = 0;
        size = sizeof length;
        if (WinHttpQueryHeaders(r, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX, &length, &size, WINHTTP_NO_HEADER_INDEX))
            g_total = length;
        if (code != 200) {
            char t[64];
            snprintf(t, sizeof t, "HTTP %lu", (unsigned long)code);
            *error = code == 404 ? "no release published yet" : t;
        } else {
            static char buf[1 << 16];
            ok = true;
            unsigned long long got_all = 0;
            for (;;) {
                DWORD got = 0;
                if (!WinHttpReadData(r, buf, sizeof buf, &got)) {
                    *error = windows_error("the connection was cut");
                    ok = false;
                    break;
                }
                if (got == 0) break;
                got_all += got;
                if (!sink(buf, got)) {
                    ok = false;
                    break;
                }
            }
            if (ok && length && got_all != length) {
                *error = "the download stopped before its end";
                ok = false;
            }
        }
    }
    if (r) WinHttpCloseHandle(r);
    if (c) WinHttpCloseHandle(c);
    WinHttpCloseHandle(s);
    return ok;
}
#else
bool http_get(const std::string &url, const char *accept, const Sink &sink, std::string *error) {
    for (char ch : url)
        if (ch == '\'' || ch == '\\') {
            *error = "bad address";
            return false;
        }
    const std::string cmd = std::string("curl -fsSL -m 30 -A RaymanRevolution-PC -H 'Accept: ") + accept + "' '" + url + "'";
    FILE *p = popen(cmd.c_str(), "r");
    if (!p) {
        *error = "curl cannot be run";
        return false;
    }
    char buf[1 << 14];
    bool ok = true;
    size_t n;
    while ((n = fread(buf, 1, sizeof buf, p)) > 0)
        if (!sink(buf, n)) { ok = false; break; }
    const int code = pclose(p);
    if (ok && code != 0) {
        *error = "no answer from GitHub (curl failed)";
        ok = false;
    }
    return ok;
}
#endif

std::string api_address() {
    return "https://api.github.com/repos/" REVO_UPDATE_REPO "/releases/latest";
}

int check_thread(void *) {
    std::string body, error;
    const bool got = http_get(api_address(), "application/vnd.github+json", [&body](const char *d, size_t n) {
        body.append(d, n);
        return body.size() < (8u << 20);
    }, &error);
    Release r;
    if (!got) {
        revo_log("[update] the check failed: %s", error.c_str());
        set_state(S_FAILED, error);
    } else if (!parse_release(body, &r) || r.tag.empty()) {
        revo_log("[update] GitHub's answer is not a release");
        set_state(S_FAILED, "GitHub's answer could not be read");
    } else {
        r.version = r.tag[0] == 'v' || r.tag[0] == 'V' ? r.tag.substr(1) : r.tag;
        if (r.page.empty()) r.page = k_releases_page;
        const bool is_newer = newer(revo_version(), r.version);
        revo_log("[update] latest release %s (this is %s)%s", r.version.c_str(), revo_version(),
                 is_newer ? ": newer" : "");
        {
            std::lock_guard<std::mutex> l(g_lock);
            g_rel = r;
            g_state = is_newer ? S_AVAILABLE : S_CURRENT;
            g_error.clear();
        }
        if (is_newer && g_at_start) g_badge_since = std::max<Uint32>(1, SDL_GetTicks());
    }
    g_at_start = false;
    g_busy = false;
    return 0;
}

void start_check(bool at_start) {
    if (g_busy.exchange(true)) return;
    g_at_start = at_start;
    set_state(S_CHECKING);
    SDL_Thread *t = SDL_CreateThread(check_thread, "update-check", nullptr);
    if (t) SDL_DetachThread(t);
    else {
        set_state(S_FAILED, "no thread");
        g_busy = false;
    }
}

#ifdef _WIN32
std::wstring exe_path() {
    std::wstring p(32768, L'\0');
    const DWORD n = GetModuleFileNameW(nullptr, &p[0], (DWORD)p.size());
    p.resize(n);
    return p;
}

std::wstring exe_dir() {
    const std::wstring p = exe_path();
    const size_t slash = p.find_last_of(L"\\/");
    return slash == std::wstring::npos ? L"." : p.substr(0, slash);
}

bool exists(const std::wstring &p) { return GetFileAttributesW(p.c_str()) != INVALID_FILE_ATTRIBUTES; }

void remove_tree(const std::wstring &dir) {
    WIN32_FIND_DATAW f;
    HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &f);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            const std::wstring n = f.cFileName;
            if (n == L"." || n == L"..") continue;
            const std::wstring p = dir + L"\\" + n;
            if ((f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(f.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
                remove_tree(p);
            } else {
                SetFileAttributesW(p.c_str(), FILE_ATTRIBUTE_NORMAL);
                DeleteFileW(p.c_str());
            }
        } while (FindNextFileW(h, &f));
        FindClose(h);
    }
    RemoveDirectoryW(dir.c_str());
}

void make_parents(const std::wstring &path) {
    for (size_t at = path.find(L'\\', 3); at != std::wstring::npos; at = path.find(L'\\', at + 1))
        CreateDirectoryW(path.substr(0, at).c_str(), nullptr);
}

bool unzip(const std::wstring &zip, const std::wstring &into, std::string *error) {
    wchar_t sys[MAX_PATH];
    GetSystemDirectoryW(sys, MAX_PATH);
    const std::wstring tar = std::wstring(sys) + L"\\tar.exe";
    if (!exists(tar)) {
        *error = "Windows' tar.exe is missing (Windows 10 1803 or later has it)";
        return false;
    }
    std::wstring cmd = L"\"" + tar + L"\" -xf \"" + zip + L"\" -C \"" + into + L"\"";
    STARTUPINFOW si = {};
    si.cb = sizeof si;
    PROCESS_INFORMATION pi = {};
    if (!CreateProcessW(tar.c_str(), &cmd[0], nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        *error = windows_error("tar.exe cannot be started");
        return false;
    }
    DWORD code = 1;
    if (WaitForSingleObject(pi.hProcess, 10 * 60 * 1000) != WAIT_OBJECT_0) TerminateProcess(pi.hProcess, 1);
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    if (code != 0) {
        *error = "the zip could not be unpacked";
        return false;
    }
    return true;
}

std::wstring package_root(const std::wstring &dir) {
    if (exists(dir + L"\\revolution.exe")) return dir;
    WIN32_FIND_DATAW f;
    HANDLE h = FindFirstFileW((dir + L"\\*").c_str(), &f);
    std::wstring found;
    if (h == INVALID_HANDLE_VALUE) return found;
    do {
        const std::wstring n = f.cFileName;
        if (n != L"." && n != L".." && (f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            exists(dir + L"\\" + n + L"\\revolution.exe"))
            found = dir + L"\\" + n;
    } while (found.empty() && FindNextFileW(h, &f));
    FindClose(h);
    return found;
}

const wchar_t *const k_pending_list = L"\\.update-old";
const wchar_t *const k_manifest = L"files.txt";

std::string sha256_file(const std::wstring &path) {
    std::string hex;
    BCRYPT_ALG_HANDLE alg = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    FILE *f = _wfopen(path.c_str(), L"rb");
    if (f && BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0)) &&
        BCRYPT_SUCCESS(BCryptCreateHash(alg, &hash, nullptr, 0, nullptr, 0, 0))) {
        static thread_local unsigned char buf[1 << 16];
        bool ok = true;
        size_t n;
        while (ok && (n = fread(buf, 1, sizeof buf, f)) > 0) ok = BCRYPT_SUCCESS(BCryptHashData(hash, buf, (ULONG)n, 0));
        unsigned char out[32];
        if (ok && !ferror(f) && BCRYPT_SUCCESS(BCryptFinishHash(hash, out, sizeof out, 0))) {
            static const char digits[] = "0123456789abcdef";
            for (unsigned char b : out) {
                hex.push_back(digits[b >> 4]);
                hex.push_back(digits[b & 15]);
            }
        }
    }
    if (hash) BCryptDestroyHash(hash);
    if (alg) BCryptCloseAlgorithmProvider(alg, 0);
    if (f) fclose(f);
    return hex;
}

struct Entry { std::wstring rel; std::string hash; };

std::wstring lower(std::wstring s) {
    if (!s.empty()) CharLowerBuffW(&s[0], (DWORD)s.size());
    return s;
}

std::wstring safe_rel(const std::string &posix) {
    if (posix.empty() || posix[0] == '/' || posix[0] == '\\' || posix.find(':') != std::string::npos) return L"";
    std::wstring rel = wide(posix);
    for (wchar_t &c : rel)
        if (c == L'/') c = L'\\';
    size_t at = 0;
    for (;;) {
        const size_t end = rel.find(L'\\', at);
        const std::wstring part = rel.substr(at, end == std::wstring::npos ? std::wstring::npos : end - at);
        if (part.empty() || part == L"." || part == L"..") return L"";
        if (end == std::wstring::npos) break;
        at = end + 1;
    }
    return rel;
}

std::vector<Entry> read_manifest(const std::wstring &path, bool *ok) {
    std::vector<Entry> out;
    *ok = true;
    FILE *f = _wfopen(path.c_str(), L"rb");
    if (!f) return out;
    char line[4096];
    while (fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (!line[0]) continue;
        const std::string s = line;
        Entry e;
        bool hex = s.size() > 66 && s[64] == ' ' && s[65] == ' ';
        for (int i = 0; hex && i < 64; i++) hex = isxdigit((unsigned char)s[i]) != 0;
        if (hex) e.rel = safe_rel(s.substr(66));
        if (!hex || e.rel.empty() || lower(e.rel) == lower(k_manifest)) {
            *ok = false;
            continue;
        }
        e.hash = s.substr(0, 64);
        for (char &c : e.hash) c = (char)tolower((unsigned char)c);
        out.push_back(e);
    }
    fclose(f);
    return out;
}

std::wstring free_name(const std::wstring &path, const wchar_t *suffix) {
    std::wstring name = path + suffix;
    for (int n = 2; exists(name); n++) name = path + suffix + std::to_wstring(n);
    return name;
}

int clean_thread(void *) {
    const std::wstring dir = exe_dir(), list = dir + k_pending_list;
    FILE *f = _wfopen(list.c_str(), L"rb");
    if (!f) return 0;
    std::vector<std::wstring> left;
    char line[4096];
    while (fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = 0;
        const std::wstring rel = safe_rel(line);
        if (!rel.empty() && lower(rel).find(L".update-old") != std::wstring::npos) left.push_back(dir + L"\\" + rel);
    }
    fclose(f);
    for (int tries = 0; tries < 40 && !left.empty(); tries++) {
        if (tries) Sleep(250);
        left.erase(std::remove_if(left.begin(), left.end(),
                                  [](const std::wstring &p) { return !exists(p) || DeleteFileW(p.c_str()); }),
                   left.end());
    }
    if (left.empty()) DeleteFileW(list.c_str());
    else if (FILE *w = _wfopen(list.c_str(), L"wb")) {
        for (const std::wstring &p : left) fprintf(w, "%s\n", narrow(p.substr(dir.size() + 1)).c_str());
        fclose(w);
    }
    revo_log("[update] set-aside files: %d left", (int)left.size());
    return 0;
}

struct Change {
    std::wstring dst;
    std::wstring aside;
    bool placed;
    bool keep;
};

void undo(const std::vector<Change> &changes) {
    for (auto c = changes.rbegin(); c != changes.rend(); ++c) {
        if (c->placed) DeleteFileW(c->dst.c_str());
        if (!c->aside.empty()) MoveFileExW(c->aside.c_str(), c->dst.c_str(), MOVEFILE_REPLACE_EXISTING);
    }
}

bool put_in_place(const std::wstring &dir, const std::wstring &root, const std::vector<Entry> &next,
                  const std::vector<Entry> &before, int *count, std::string *error) {
    auto old_hash = [&before](const std::wstring &rel) -> const std::string * {
        const std::wstring key = lower(rel);
        for (const Entry &e : before)
            if (lower(e.rel) == key) return &e.hash;
        return nullptr;
    };
    std::vector<Change> changes;
    bool ok = true;
    int kept = 0;
    for (const Entry &e : next) {
        const std::wstring dst = dir + L"\\" + e.rel, src = root + L"\\" + e.rel;
        Change c{dst, L"", false, false};
        if (exists(dst)) {
            const std::string now = sha256_file(dst);
            if (now == e.hash) continue;
            const std::string *was = old_hash(e.rel);
            c.keep = !was || now.empty() || now != *was;
            c.aside = free_name(dst, c.keep ? L".bak" : L".update-old");
            if (!MoveFileExW(dst.c_str(), c.aside.c_str(), 0)) {
                *error = windows_error(("cannot move " + narrow(e.rel) + " aside").c_str());
                ok = false;
                break;
            }
            if (c.keep) {
                kept++;
                revo_log("[update] %s is not as installed: kept as %s", narrow(e.rel).c_str(),
                         narrow(c.aside.substr(dir.size() + 1)).c_str());
            }
        } else {
            make_parents(dst);
        }
        if (!MoveFileExW(src.c_str(), dst.c_str(), MOVEFILE_COPY_ALLOWED)) {
            *error = windows_error(("cannot put " + narrow(e.rel) + " in place").c_str());
            if (!c.aside.empty()) MoveFileExW(c.aside.c_str(), dst.c_str(), 0);
            ok = false;
            break;
        }
        c.placed = true;
        changes.push_back(c);
    }
    for (const Entry &e : before) {
        if (!ok) break;
        bool still = false;
        for (const Entry &n : next) still = still || lower(n.rel) == lower(e.rel);
        const std::wstring dst = dir + L"\\" + e.rel;
        if (still || !exists(dst) || sha256_file(dst) != e.hash) continue;
        Change c{dst, free_name(dst, L".update-old"), false, false};
        if (!MoveFileExW(dst.c_str(), c.aside.c_str(), 0)) continue;
        changes.push_back(c);
    }
    if (ok) {
        const std::wstring list = dir + L"\\" + k_manifest;
        Change c{list, exists(list) ? free_name(list, L".update-old") : L"", false, false};
        if (!c.aside.empty() && !MoveFileExW(list.c_str(), c.aside.c_str(), 0)) {
            *error = windows_error("cannot move files.txt aside");
            ok = false;
        } else if (!MoveFileExW((root + L"\\" + k_manifest).c_str(), list.c_str(), MOVEFILE_COPY_ALLOWED)) {
            *error = windows_error("cannot put files.txt in place");
            if (!c.aside.empty()) MoveFileExW(c.aside.c_str(), list.c_str(), 0);
            ok = false;
        } else {
            c.placed = true;
            changes.push_back(c);
        }
    }
    if (!ok) {
        undo(changes);
        revo_log("[update] install failed, the folder is as it was: %s", error->c_str());
        return false;
    }
    std::string pending;
    for (const Change &c : changes)
        if (!c.aside.empty() && !c.keep && !DeleteFileW(c.aside.c_str()))
            pending += narrow(c.aside.substr(dir.size() + 1)) + "\n";
    if (!pending.empty()) {
        if (FILE *l = _wfopen((dir + k_pending_list).c_str(), L"ab")) {
            fwrite(pending.data(), 1, pending.size(), l);
            fclose(l);
        }
    }
    *count = (int)changes.size();
    if (kept) revo_log("[update] %d file(s) of yours kept as .bak", kept);
    return true;
}

bool install(std::string *error) {
    Release r;
    {
        std::lock_guard<std::mutex> l(g_lock);
        r = g_rel;
    }
    if (r.zip.empty()) {
        *error = "this release has no Windows zip";
        return false;
    }
    const std::wstring dir = exe_dir();
    {
        HANDLE h = CreateFileW((dir + L"\\.update-probe").c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                               FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
        if (h == INVALID_HANDLE_VALUE) {
            *error = "the game's folder cannot be written to: unzip the release over it by hand";
            return false;
        }
        CloseHandle(h);
    }
    wchar_t tmp[MAX_PATH + 1];
    const DWORD tn = GetTempPathW(MAX_PATH + 1, tmp);
    if (tn == 0 || tn > MAX_PATH) {
        *error = "Windows gives no temporary folder";
        return false;
    }
    const std::wstring work = std::wstring(tmp, tn) + L"RaymanRevolution-update";
    remove_tree(work);
    if (!CreateDirectoryW(work.c_str(), nullptr)) {
        *error = windows_error("no temporary folder");
        return false;
    }
    const std::wstring zip = work + L"\\package.zip";
    FILE *f = _wfopen(zip.c_str(), L"wb");
    if (!f) {
        *error = "the download cannot be written in the temporary folder";
        remove_tree(work);
        return false;
    }
    g_done = 0;
    g_total = 0;
    revo_log("[update] downloading %s", r.zip.c_str());
    bool ok = http_get(r.zip, "application/octet-stream", [f](const char *d, size_t n) {
        if (g_cancel) return false;
        g_done += n;
        return fwrite(d, 1, n, f) == n;
    }, error);
    if (fclose(f) != 0 && ok) {
        ok = false;
        *error = "the download could not be written (is the disk full?)";
    }
    if (!ok) {
        if (g_cancel) *error = "cancelled";
        else if (error->empty()) *error = "the download failed";
        remove_tree(work);
        return false;
    }
    set_state(S_INSTALLING);
    const std::wstring files = work + L"\\files";
    CreateDirectoryW(files.c_str(), nullptr);
    if (!unzip(zip, files, error)) {
        remove_tree(work);
        return false;
    }
    const std::wstring root = package_root(files);
    bool next_ok = false, before_ok = false;
    std::vector<Entry> next;
    if (!root.empty()) next = read_manifest(root + L"\\" + k_manifest, &next_ok);
    if (root.empty() || next.empty() || !next_ok) {
        *error = "the zip does not hold the program and its files.txt";
        remove_tree(work);
        return false;
    }
    for (const Entry &e : next) {
        if (sha256_file(root + L"\\" + e.rel) != e.hash) {
            *error = "the zip's " + narrow(e.rel) + " is not the one its files.txt names";
            remove_tree(work);
            return false;
        }
    }
    const std::vector<Entry> before = read_manifest(dir + L"\\" + k_manifest, &before_ok);
    if (!before_ok) revo_log("[update] the installed files.txt has lines that are not files: they are left alone");
    int count = 0;
    ok = put_in_place(dir, root, next, before, &count, error);
    remove_tree(work);
    if (ok) revo_log("[update] %s installed (%d files changed)", r.version.c_str(), count);
    return ok;
}

void restart_now() {
    g_restart = true;
    SDL_Event quit = {};
    quit.type = SDL_QUIT;
    SDL_PushEvent(&quit);
}

int install_thread(void *) {
    std::string error;
    const bool ok = install(&error);
    if (!ok) revo_log("[update] not installed: %s", error.c_str());
    set_state(ok ? S_INSTALLED : S_INSTALL_FAILED, error);
    g_cancel = false;
    g_busy = false;
    return 0;
}

void start_install() {
    if (g_busy.exchange(true)) return;
    g_cancel = false;
    set_state(S_DOWNLOADING);
    SDL_Thread *t = SDL_CreateThread(install_thread, "update-install", nullptr);
    if (t) SDL_DetachThread(t);
    else {
        set_state(S_INSTALL_FAILED, "no thread");
        g_busy = false;
    }
}

void restart_at_exit() {
    if (!g_restart) return;
    const std::wstring exe = exe_path();
    std::wstring cmd = GetCommandLineW();
    STARTUPINFOW si = {};
    si.cb = sizeof si;
    PROCESS_INFORMATION pi = {};
    if (CreateProcessW(exe.c_str(), &cmd[0], nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
}
#endif

std::string megabytes(unsigned long long n) {
    char t[32];
    snprintf(t, sizeof t, "%.1f MB", n / 1048576.0);
    return t;
}

}

extern "C" void revo_update_start(void) {
#ifdef _WIN32
    atexit(restart_at_exit);
    SDL_Thread *t = SDL_CreateThread(clean_thread, "update-clean", nullptr);
    if (t) SDL_DetachThread(t);
#endif
    const char *e = getenv("REVO_UPDATE");
    if (e && (!strcmp(e, "off") || !strcmp(e, "0"))) return;
    int v[3];
    if (revo_set(REVO_SET_UPDATE_CHECK) && parse_version(revo_version(), v)) start_check(true);
}

void revo_update_menu(void) {
    State state;
    Release r;
    std::string error;
    {
        std::lock_guard<std::mutex> l(g_lock);
        state = g_state;
        r = g_rel;
        error = g_error;
    }
    const ImVec4 good(0.55f, 0.85f, 0.45f, 1.0f), warn(1.0f, 0.75f, 0.25f, 1.0f);
    ImGui::TextDisabled("This version: %s", revo_version());
    ImGui::PushTextWrapPos(0.0f);
    switch (state) {
    case S_IDLE: break;
    case S_CHECKING: ImGui::TextUnformatted("Checking..."); break;
    case S_CURRENT: ImGui::TextColored(good, "This is the latest version (%s).", r.version.c_str()); break;
    case S_FAILED: ImGui::TextColored(warn, "The check failed: %s.", error.c_str()); break;
    case S_AVAILABLE:
    case S_INSTALL_FAILED:
        ImGui::TextColored(good, "Version %s is out.", r.version.c_str());
        if (state == S_INSTALL_FAILED)
            ImGui::TextColored(warn, "The update failed: %s. Nothing has been changed.", error.c_str());
        break;
    case S_DOWNLOADING: {
        const unsigned long long done = g_done, total = g_total;
        const std::string label = megabytes(done) + (total ? " of " + megabytes(total) : std::string());
        ImGui::TextUnformatted(("Downloading " + r.version + "...").c_str());
        ImGui::ProgressBar(total ? (float)((double)done / (double)total) : 0.0f, ImVec2(-FLT_MIN, 0), label.c_str());
        break;
    }
    case S_INSTALLING: ImGui::TextUnformatted("Installing..."); break;
    case S_INSTALLED:
        ImGui::TextColored(good, "Version %s is installed: restart to see the change.", r.version.c_str());
        ImGui::TextDisabled("Restarting now might mess up what was not saved.");
        break;
    }
    if ((state == S_AVAILABLE || state == S_INSTALL_FAILED) && !r.notes.empty()) {
        const float s = revo_ui_scale();
        ImGui::BeginChild("notes", ImVec2(-FLT_MIN, 110 * s), ImGuiChildFlags_Borders);
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextUnformatted(r.notes.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndChild();
    }
    ImGui::PopTextWrapPos();

    if (state == S_AVAILABLE || state == S_INSTALL_FAILED) {
#ifdef _WIN32
        if (!r.zip.empty()) {
            if (ImGui::Button(state == S_AVAILABLE ? "Download and install" : "Try again!")) start_install();
            ImGui::SameLine();
        }
#endif
        if (ImGui::Button("Releases page")) SDL_OpenURL(r.page.empty() ? k_releases_page : r.page.c_str());
        ImGui::SameLine();
    }
#ifdef _WIN32
    if (state == S_DOWNLOADING && ImGui::Button("Cancel")) g_cancel = true;
    if (state == S_INSTALLED) {
        if (ImGui::Button("Restart now")) restart_now();
        ImGui::SameLine();
    }
#endif
    if (state != S_DOWNLOADING && state != S_INSTALLING && state != S_INSTALLED) {
        ImGui::BeginDisabled(state == S_CHECKING);
        if (ImGui::Button("Check now")) start_check(false);
        ImGui::EndDisabled();
    }
}

bool revo_update_badge_wanted(void) {
    const Uint32 since = g_badge_since;
    if (!since) return false;
    if (SDL_GetTicks() - since < 12000) return true;
    g_badge_since = 0;
    return false;
}

void revo_update_badge(void) {
    std::string version;
    {
        std::lock_guard<std::mutex> l(g_lock);
        version = g_rel.version;
    }
    const std::string t = "Rayman Re-Revolution " + version + " is out: F1 > System to update";
    ImDrawList *d = ImGui::GetForegroundDrawList();
    const float s = revo_ui_scale(), pad = 4 * s;
    const ImVec2 size = ImGui::CalcTextSize(t.c_str());
    const ImVec2 at(ImGui::GetIO().DisplaySize.x - 8 * s - size.x, 8 * s);
    d->AddRectFilled(ImVec2(at.x - pad, at.y - pad), ImVec2(at.x + size.x + pad, at.y + size.y + pad),
                     IM_COL32(0, 0, 0, 170), 4 * s);
    d->AddText(at, IM_COL32(140, 220, 110, 255), t.c_str());
}
