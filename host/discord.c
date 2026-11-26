#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#include "mods.h"
#include "runtime.h"

#ifndef REVO_DISCORD_APP_ID
#define REVO_DISCORD_APP_ID "1573711607798702191"
#endif

const char *revo_current_level(void);
void revo_progress(int *lums, int *cages);

enum { OP_HANDSHAKE, OP_FRAME, OP_CLOSE, OP_PING, OP_PONG };

#ifdef _WIN32
static HANDLE g_pipe = INVALID_HANDLE_VALUE;
#else
static int g_sock = -1;
#endif
static char g_app_id[32];

static int connected(void) {
#ifdef _WIN32
    return g_pipe != INVALID_HANDLE_VALUE;
#else
    return g_sock >= 0;
#endif
}

static void disconnect(void) {
#ifdef _WIN32
    if (g_pipe != INVALID_HANDLE_VALUE) CloseHandle(g_pipe);
    g_pipe = INVALID_HANDLE_VALUE;
#else
    if (g_sock >= 0) close(g_sock);
    g_sock = -1;
#endif
}

static int open_ipc(void) {
#ifdef _WIN32
    for (int n = 0; n < 10; n++) {
        char name[64];
        snprintf(name, sizeof name, "\\\\.\\pipe\\discord-ipc-%d", n);
        g_pipe = CreateFileA(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (g_pipe != INVALID_HANDLE_VALUE) return 1;
    }
    return 0;
#else
    const char *bases[] = {getenv("XDG_RUNTIME_DIR"), getenv("TMPDIR"), getenv("TMP"), getenv("TEMP"), "/tmp"};
    const char *subs[] = {"", "app/com.discordapp.Discord/", "snap.discord/", ".flatpak/dev.vencord.Vesktop/xdg-run/"};
    for (size_t b = 0; b < sizeof bases / sizeof bases[0]; b++) {
        if (!bases[b] || !*bases[b]) continue;
        for (size_t s = 0; s < sizeof subs / sizeof subs[0]; s++) {
            for (int n = 0; n < 10; n++) {
                struct sockaddr_un addr;
                memset(&addr, 0, sizeof addr);
                addr.sun_family = AF_UNIX;
                const size_t len = strlen(bases[b]);
                const char *sep = len && bases[b][len - 1] == '/' ? "" : "/";
                if (snprintf(addr.sun_path, sizeof addr.sun_path, "%s%s%sdiscord-ipc-%d", bases[b], sep, subs[s], n) >=
                    (int)sizeof addr.sun_path)
                    continue;
                const int fd = socket(AF_UNIX, SOCK_STREAM, 0);
                if (fd < 0) return 0;
#ifdef SO_NOSIGPIPE
                const int one = 1;
                setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof one);
#endif
                if (connect(fd, (struct sockaddr *)&addr, sizeof addr) == 0) {
                    g_sock = fd;
                    return 1;
                }
                close(fd);
            }
        }
    }
    return 0;
#endif
}

static int write_all(const void *data, size_t size) {
    const char *p = (const char *)data;
#ifdef _WIN32
    while (size) {
        DWORD wrote = 0;
        if (!WriteFile(g_pipe, p, (DWORD)size, &wrote, NULL) || !wrote) return 0;
        p += wrote;
        size -= wrote;
    }
#else
#ifdef MSG_NOSIGNAL
    const int flags = MSG_NOSIGNAL;
#else
    const int flags = 0;
#endif
    while (size) {
        const ssize_t wrote = send(g_sock, p, size, flags);
        if (wrote <= 0) return 0;
        p += wrote;
        size -= (size_t)wrote;
    }
#endif
    return 1;
}

static int send_frame(unsigned op, const char *json) {
    const unsigned len = (unsigned)strlen(json);
    unsigned char head[8];
    for (int k = 0; k < 4; k++) {
        head[k] = (unsigned char)(op >> (8 * k));
        head[4 + k] = (unsigned char)(len >> (8 * k));
    }
    if (write_all(head, sizeof head) && write_all(json, len)) return 1;
    disconnect();
    return 0;
}

static long available(void) {
#ifdef _WIN32
    DWORD n = 0;
    if (!PeekNamedPipe(g_pipe, NULL, 0, NULL, &n, NULL)) return -1;
    return (long)n;
#else
    char c;
    const ssize_t n = recv(g_sock, &c, 1, MSG_PEEK | MSG_DONTWAIT);
    if (n > 0) return 1;
    if (n == 0) return -1;
    return errno == EAGAIN || errno == EWOULDBLOCK ? 0 : -1;
#endif
}

static int read_all(void *data, size_t size) {
    char *p = (char *)data;
    while (size) {
#ifdef _WIN32
        DWORD got = 0;
        if (!ReadFile(g_pipe, p, (DWORD)size, &got, NULL) || !got) return 0;
#else
        const ssize_t got = recv(g_sock, p, size, 0);
        if (got <= 0) return 0;
#endif
        p += got;
        size -= (size_t)got;
    }
    return 1;
}

static int read_frame(unsigned *op, char *json, size_t size) {
    unsigned char head[8];
    if (!read_all(head, sizeof head)) return 0;
    *op = head[0] | head[1] << 8 | head[2] << 16 | (unsigned)head[3] << 24;
    unsigned len = head[4] | head[5] << 8 | head[6] << 16 | (unsigned)head[7] << 24;
    size_t kept = 0;
    while (len) {
        char chunk[512];
        const unsigned n = len < sizeof chunk ? len : (unsigned)sizeof chunk;
        if (!read_all(chunk, n)) return 0;
        if (kept + 1 < size) {
            const size_t take = n < size - 1 - kept ? n : size - 1 - kept;
            memcpy(json + kept, chunk, take);
            kept += take;
        }
        len -= n;
    }
    if (size) json[kept] = 0;
    return 1;
}

static int pump(void) {
    char json[1024];
    for (;;) {
        const long n = available();
        if (n < 0) {
            disconnect();
            return 0;
        }
        if (n == 0) return 1;
        unsigned op;
        if (!read_frame(&op, json, sizeof json)) {
            disconnect();
            return 0;
        }
        if (op == OP_PING) {
            if (!send_frame(OP_PONG, json)) return 0;
        } else if (op == OP_CLOSE) {
            revo_log("[discord] closed by Discord: %s", json);
            disconnect();
            return 0;
        } else if (strstr(json, "\"evt\":\"ERROR\"")) {
            revo_log("[discord] %s", json);
        }
    }
}

static int handshake(void) {
    char json[128];
    snprintf(json, sizeof json, "{\"v\":1,\"client_id\":\"%s\"}", g_app_id);
    if (!send_frame(OP_HANDSHAKE, json)) return 0;
    unsigned op;
    char reply[1024];
    if (!read_frame(&op, reply, sizeof reply)) {
        disconnect();
        return 0;
    }
    if (op != OP_FRAME || !strstr(reply, "\"READY\"")) {
        revo_log("[discord] refused: %s", reply);
        disconnect();
        return 0;
    }
    revo_log("[discord] connected");
    return 1;
}

static long process_id(void) {
#ifdef _WIN32
    return (long)GetCurrentProcessId();
#else
    return (long)getpid();
#endif
}

typedef struct { const char *prefix, *place, *art; } Place;

static const Place k_places[] = {
    {"jail_", "The Prison Ship", "prison_ship"},
    {"learn_10", "The Woods of Light", "woods_of_light"},
    {"learn_30", "The Fairy Glade", "fairy_glade"},
    {"learn_31", "The Fairy Glade", "fairy_glade"},
    {"learn_60", "The Fairy Glade", "fairy_glade"},
    {"bast_20", "The Fairy Glade", "fairy_glade"},
    {"bast_22", "The Fairy Glade", "fairy_glade"},
    {"bast_10", "The Echoing Caves", "echoing_caves"},
    {"cask_", "The Echoing Caves", "echoing_caves"},
    {"ski_", "The Marshes of Awakening", "marshes"},
    {"chase_", "The Bayou", "bayou"},
    {"water_", "The Sanctuary of Water and Ice", "water_ice"},
    {"rodeo_", "The Menhir Hills", "menhir_hills"},
    {"vulca_", "The Cave of Bad Dreams", "bad_dreams"},
    {"glob_", "The Canopy", "canopy"},
    {"whale_", "Whale Bay", "whale_bay"},
    {"plum_", "The Sanctuary of Stone and Fire", "stone_fire"},
    {"nave_", "The Precipice", "precipice"},
    {"seat_", "The Top of the World", "top_of_world"},
    {"earth_", "The Sanctuary of Rock and Lava", "rock_lava"},
    {"helic_", "Beneath the Sanctuary of Rock and Lava", "beneath_rock_lava"},
    {"ly_10", "The Walk of Life", "walk_of_life"},
    {"ly_20", "The Walk of Power", "walk_of_power"},
    {"morb_", "The Tomb of the Ancients", "tomb"},
    {"learn_40", "The Iron Mountains", "iron_mountains"},
    {"ile_", "The Iron Mountains", "iron_mountains"},
    {"mine_", "The Iron Mountains", "iron_mountains"},
    {"boat", "The Iron Mountains", "iron_mountains"},
    {"astro_", "The Prison Ship", "prison_ship"},
    {"rhop_", "The Crow's Nest", "crows_nest"},
    {"mc_", "The Glade of Dreams", "glade_of_dreams"},
    {"b_", "A bonus game", "bonus"},
    {"bonux", "A bonus game", "bonus"},
    {"end_", "The end", "logo"},
};

static const Place *place_of(const char *level) {
    for (size_t k = 0; k < sizeof k_places / sizeof k_places[0]; k++)
        if (!strncasecmp(level, k_places[k].prefix, strlen(k_places[k].prefix))) return &k_places[k];
    return NULL;
}

static int ignored(const char *level) {
    return !*level || !strcasecmp(level, "fix") || !strcasecmp(level, "vignette");
}

static void progress_text(char *out, size_t size) {
    int lums = -1, cages = -1;
    revo_progress(&lums, &cages);
    out[0] = 0;
    if (lums >= 0 && cages >= 0)
        snprintf(out, size, "Lums %d/1000 \xc2\xb7 Cages %d/80", lums, cages);
}

static int send_activity(const char *level, const char *progress) {
    static unsigned nonce;
    static const char *world;
    static long long since;
    char json[1024];
    if (!strncasecmp(level, "menu", 4)) {
        world = NULL;
        snprintf(json, sizeof json,
                 "{\"cmd\":\"SET_ACTIVITY\",\"nonce\":\"%u\",\"args\":{\"pid\":%ld,\"activity\":{"
                 "\"details\":\"In the menus\",\"assets\":{\"large_image\":\"logo\","
                 "\"large_text\":\"Rayman Re-Revolution\"}}}}",
                 ++nonce, process_id());
        revo_log("[discord] in the menus");
        return send_frame(OP_FRAME, json);
    }
    const Place *place = place_of(level);
    const char *name = place ? place->place : "Playing";
    if (name != world) {
        world = name;
        since = (long long)time(NULL);
    }
    char state[160] = "";
    if (*progress) snprintf(state, sizeof state, "\"state\":\"%s\",", progress);
    snprintf(json, sizeof json,
             "{\"cmd\":\"SET_ACTIVITY\",\"nonce\":\"%u\",\"args\":{\"pid\":%ld,\"activity\":{"
             "\"details\":\"%s\",%s\"timestamps\":{\"start\":%lld},"
             "\"assets\":{\"large_image\":\"%s\",\"large_text\":\"%s\","
             "\"small_image\":\"logo\",\"small_text\":\"Rayman Re-Revolution\"}}}}",
             ++nonce, process_id(), name, state, since, place ? place->art : "logo", name);
    revo_log("[discord] %s (%s) %s", name, level, progress);
    return send_frame(OP_FRAME, json);
}

static int SDLCALL presence_thread(void *unused) {
    (void)unused;
    char shown[256] = "", level[64] = "";
    Uint64 retry_at = 0, sent_at = 0;
    for (;; SDL_Delay(5000)) {
        if (!revo_set(REVO_SET_DISCORD)) {
            if (connected()) {
                revo_log("[discord] off");
                disconnect();
            }
            shown[0] = 0;
            continue;
        }
        const char *now = revo_current_level();
        char copy[64];
        size_t k = 0;
        for (; k < sizeof copy - 1 && now[k]; k++) copy[k] = now[k];
        copy[k] = 0;
        if (!ignored(copy)) snprintf(level, sizeof level, "%s", copy);
        const Uint64 t = SDL_GetTicks64();
        if (!connected()) {
            shown[0] = 0;
            if (t < retry_at) continue;
            retry_at = t + 15000;
            if (!open_ipc() || !handshake()) continue;
        }
        if (!pump()) continue;
        const char *want = level[0] ? level : "menu";
        char progress[128], key[256];
        progress_text(progress, sizeof progress);
        snprintf(key, sizeof key, "%s|%s", want, progress);
        if (!strcmp(key, shown) || t < sent_at + 5000) continue;
        if (!send_activity(want, progress)) continue;
        snprintf(shown, sizeof shown, "%s", key);
        sent_at = t;
    }
    return 0;
}

void revo_discord_start(void) {
    snprintf(g_app_id, sizeof g_app_id, "%s", REVO_DISCORD_APP_ID);
    if (!g_app_id[0]) return;
    for (const char *p = g_app_id; *p; p++)
        if (*p < '0' || *p > '9') {
            revo_log("[discord] the application id %s is not a number", g_app_id);
            return;
        }
    SDL_Thread *thread = SDL_CreateThread(presence_thread, "discord", NULL);
    if (thread) SDL_DetachThread(thread);
}

int revo_discord_available(void) {
    return sizeof REVO_DISCORD_APP_ID > 1;
}
