#include "game_vars.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game_api.h"
#include "glue_rt.h"
#include "runtime.h"
#include "native_mem.h"

void revo_kernel_init(void);
void revo_kernel_start(void);

static u32 rd_le32(const u8 *p) { return p[0] | p[1] << 8 | p[2] << 16 | (u32)p[3] << 24; }
static u16 rd_le16(const u8 *p) { return (u16)(p[0] | p[1] << 8); }

u32 revo_program_end;

static u32 load_elf(void) {
    u32 n = 0;
    u8 *d = revo_disc_load("SLUS_201.38", &n);
    if (!d) { fprintf(stderr, "cannot read SLUS_201.38 from the game's disc\n"); exit(1); }
    if (n < 52 || memcmp(d, "\x7f" "ELF", 4)) { fprintf(stderr, "SLUS_201.38 is not an ELF\n"); exit(1); }
    u32 entry = rd_le32(d + 24), phoff = rd_le32(d + 28);
    u16 phentsize = rd_le16(d + 42), phnum = rd_le16(d + 44);
    for (int i = 0; i < phnum; i++) {
        const u8 *ph = d + phoff + (u32)i * phentsize;
        if (rd_le32(ph) != 1) continue;
        u32 off = rd_le32(ph + 4), va = rd_le32(ph + 8), fsz = rd_le32(ph + 16), msz = rd_le32(ph + 20);
        memcpy(RAM + va, d + off, fsz);
        memset(RAM + va + fsz, 0, msz - fsz);
        if (va + msz > revo_program_end) revo_program_end = va + msz;
        revo_log("[elf] PT_LOAD 0x%08X file 0x%X mem 0x%X", va, fsz, msz);
    }
    free(d);
    return entry;
}

#include <pthread.h>
#include <signal.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
static int last_build_assets(char *out, size_t size) {
    char path[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, path, sizeof path);
    if (n == 0 || n >= sizeof path) return 0;
    char *slash = strrchr(path, '\\');
    if (!slash || (size_t)(slash - path) + sizeof "\\build_info.json" > sizeof path) return 0;
    strcpy(slash, "\\build_info.json");
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    char text[4096];
    size_t len = fread(text, 1, sizeof text - 1, f);
    fclose(f);
    text[len] = 0;
    const char *p = strstr(text, "\"assets\"");
    if (!p || !(p = strchr(p + 8, '"'))) return 0;
    size_t k = 0;
    for (p++; *p && *p != '"' && k + 1 < size; p++) {
        if (*p == '\\' && p[1]) p++;
        out[k++] = *p;
    }
    out[k] = 0;
    return *p == '"' && k > 0;
}
#else
#include <unistd.h>
#endif
#if defined(__APPLE__) || defined(__GLIBC__)
#include <execinfo.h>
#define REVO_BACKTRACE 1
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
void revo_native_backtrace(void) { extern void dump_native_stack_pub(void); dump_native_stack_pub(); }
static void dump_native_stack(int sig) {
    (void)sig;
#ifdef REVO_BACKTRACE
    void *bt[64];
    int n = backtrace(bt, 64);
#ifdef __APPLE__
    fprintf(stderr, "[native] slide 0x%lx\n", (long)_dyld_get_image_vmaddr_slide(0));
#endif
    for (int i = 0; i < n; i++) fprintf(stderr, "[native] %p\n", bt[i]);
#elif defined(_WIN32)
    void *bt[64];
    USHORT n = CaptureStackBackTrace(0, 64, bt, NULL);
    const u8 *base = (const u8 *)GetModuleHandleA(NULL);
    for (USHORT i = 0; i < n; i++) {
        const u8 *pc = (const u8 *)bt[i];
        HMODULE m = NULL;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCSTR)pc, &m);
        if ((const u8 *)m == base) fprintf(stderr, "[native] rva 0x%llx\n", (unsigned long long)(pc - base));
        else fprintf(stderr, "[native] %p (another module)\n", (void *)pc);
    }
#else
    fprintf(stderr, "[native] no stack trace on this system\n");
#endif
}
void dump_native_stack_pub(void) { dump_native_stack(0); }

#ifdef _WIN32
static int ai_state(char *buf, int n, const CONTEXT *c) {
    const u32 rdx = (u32)c->Rdx;
    if (rdx >= 0x10010 && rdx < 0x7FFFFFF0 && !IsBadReadPtr(RAM + rdx - 16, 32)) {
        const u32 *w = (const u32 *)(RAM + rdx - 16);
        n += snprintf(buf + n, 2048 - n, "\n[fault] at rdx-16: %08X %08X %08X %08X | %08X %08X %08X %08X", w[0], w[1],
                      w[2], w[3], w[4], w[5], w[6], w[7]);
    }
    {
        const u32 so = rd32(GAME_VAR(g_hCurrentSuperObjPerso));
        const u32 in = rd32(GAME_VAR(g_p_stIntelligence));
        n += snprintf(buf + n, 2048 - n, "\n[fault] AI: actor %08X intelligence %08X rule %u", so, in,
                      rd8(GAME_VAR(g_ucNRule)));
        if (in >= 0x10000 && !IsBadReadPtr(RAM + in, 24)) {
            const u32 *w = (const u32 *)(RAM + in);
            n += snprintf(buf + n, 2048 - n, ": script %08X schedule %08X comport %08X prev %08X actions %08X",
                          w[0], w[1], w[2], w[3], w[4]);
            if (w[2] >= 0x10000 && !IsBadReadPtr(RAM + w[2], 12)) {
                const u32 *cp = (const u32 *)(RAM + w[2]);
                n += snprintf(buf + n, 2048 - n, " | comport rules %08X schedule %08X count %u", cp[0], cp[1],
                              cp[2] & 0xFF);
            }
            if (w[4] >= 0x10000 && !IsBadReadPtr(RAM + w[4], 8)) {
                const u32 *at = (const u32 *)(RAM + w[4]);
                n += snprintf(buf + n, 2048 - n, " | action table %08X entries %u used %u current %u", at[0],
                              at[1] & 0xFF, at[1] >> 8 & 0xFF, at[1] >> 16 & 0xFF);
            }
        }
    }
    return n;
}

static LONG WINAPI on_native_fault(EXCEPTION_POINTERS *e) {
    const DWORD code = e->ExceptionRecord->ExceptionCode;
    if (code != EXCEPTION_ACCESS_VIOLATION && code != EXCEPTION_ILLEGAL_INSTRUCTION &&
        code != EXCEPTION_INT_DIVIDE_BY_ZERO && code != EXCEPTION_STACK_OVERFLOW && code != EXCEPTION_PRIV_INSTRUCTION)
        return EXCEPTION_CONTINUE_SEARCH;
    static char buf[2048];
    const uintptr_t base = (uintptr_t)GetModuleHandleA(NULL), link = 0x200000000ull;
    const IMAGE_NT_HEADERS *nt = (const IMAGE_NT_HEADERS *)(base + ((const IMAGE_DOS_HEADER *)base)->e_lfanew);
    const uintptr_t end = base + nt->OptionalHeader.SizeOfImage;
    const CONTEXT *c = e->ContextRecord;
    const uintptr_t at = (uintptr_t)e->ExceptionRecord->ExceptionAddress;
    if (at < base || at >= end) {
        HMODULE m = NULL;
        char path[MAX_PATH] = "?";
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               (LPCSTR)at, &m))
            GetModuleFileNameA(m, path, sizeof path);
        ULONG_PTR lo = 0, hi = 0;
        GetCurrentThreadStackLimits(&lo, &hi);
        int n = snprintf(buf, sizeof buf, "[fault] %08lX in %s+%llX; the program's addresses on the stack:",
                         (unsigned long)code, path, (unsigned long long)(at - (uintptr_t)m));
        const uintptr_t *sp = (const uintptr_t *)c->Rsp;
        for (int k = 0, found = 0; (uintptr_t)(sp + k + 1) <= hi && k < 8192 && found < 16 && n < (int)sizeof buf - 24; k++)
            if (sp[k] >= base && sp[k] < end) {
                n += snprintf(buf + n, sizeof buf - n, " %llX", (unsigned long long)(sp[k] - base + link));
                found++;
            }
        n += snprintf(buf + n, sizeof buf - n, "\n[fault] rax %llX rcx %llX rdx %llX r8 %llX r9 %llX rbx %llX rsi %llX rdi %llX",
                      (unsigned long long)c->Rax, (unsigned long long)c->Rcx, (unsigned long long)c->Rdx,
                      (unsigned long long)c->R8, (unsigned long long)c->R9, (unsigned long long)c->Rbx,
                      (unsigned long long)c->Rsi, (unsigned long long)c->Rdi);
        n = ai_state(buf, n, c);
        n += snprintf(buf + n, sizeof buf - n, "\n[fault] rsp %llX (stack %llX-%llX):", (unsigned long long)c->Rsp,
                      (unsigned long long)lo, (unsigned long long)hi);
        for (int k = 0; k < 12 && (uintptr_t)(sp + k + 1) <= hi && (uintptr_t)(sp + k) >= lo; k++) {
            HMODULE mk = NULL;
            char pk[MAX_PATH] = "";
            if (sp[k] > 0x10000 && GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                                     GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)sp[k], &mk)) {
                GetModuleFileNameA(mk, pk, sizeof pk);
                const char *slash = strrchr(pk, '\\');
                n += snprintf(buf + n, sizeof buf - n, " %llX(%s+%llX)", (unsigned long long)sp[k], slash ? slash + 1 : pk,
                              (unsigned long long)(sp[k] - (uintptr_t)mk));
            } else
                n += snprintf(buf + n, sizeof buf - n, " %llX", (unsigned long long)sp[k]);
        }
        n += snprintf(buf + n, sizeof buf - n, "\n");
        DWORD w;
        WriteFile(GetStdHandle(STD_ERROR_HANDLE), buf, (DWORD)n, &w, NULL);
        revo_log_crash(buf, (size_t)n);
        return EXCEPTION_CONTINUE_SEARCH;
    }
    int n = snprintf(buf, sizeof buf, "[crash] exception %08lX at %llX (link %llX), %s 0x%llX\n"
                     "[crash] rax %llX rcx %llX rdx %llX r8 %llX r9 %llX rsi %llX rdi %llX rsp %llX\n[crash] stack:",
                     (unsigned long)code, (unsigned long long)at,
                     (unsigned long long)(at >= base && at < end ? at - base + link : 0),
                     e->ExceptionRecord->ExceptionInformation[0] ? "writing" : "reading",
                     (unsigned long long)e->ExceptionRecord->ExceptionInformation[1], (unsigned long long)c->Rax,
                     (unsigned long long)c->Rcx, (unsigned long long)c->Rdx, (unsigned long long)c->R8,
                     (unsigned long long)c->R9, (unsigned long long)c->Rsi, (unsigned long long)c->Rdi,
                     (unsigned long long)c->Rsp);
    ULONG_PTR lo = 0, hi = 0;
    GetCurrentThreadStackLimits(&lo, &hi);
    const uintptr_t *sp = (const uintptr_t *)c->Rsp;
    for (int k = 0, found = 0; (uintptr_t)(sp + k + 1) <= hi && k < 4096 && found < 24 && n < (int)sizeof buf - 24; k++)
        if (sp[k] >= base && sp[k] < end) {
            n += snprintf(buf + n, sizeof buf - n, " %llX", (unsigned long long)(sp[k] - base + link));
            found++;
        }
    n = ai_state(buf, n, c);
    n += snprintf(buf + n, sizeof buf - n, "\n");
    DWORD w;
    WriteFile(GetStdHandle(STD_ERROR_HANDLE), buf, (DWORD)n, &w, NULL);
    revo_log_crash(buf, (size_t)n);
    TerminateProcess(GetCurrentProcess(), 128);
    return EXCEPTION_CONTINUE_SEARCH;
}
static LONG WINAPI on_crash(EXCEPTION_POINTERS *e) {
    revo_log("[crash] exception %08lX at host address %p", (unsigned long)e->ExceptionRecord->ExceptionCode,
             e->ExceptionRecord->ExceptionAddress);
    revo_dump_ring();
    _exit(128);
    return EXCEPTION_EXECUTE_HANDLER;
}
#else
static void on_crash(int sig, siginfo_t *si, void *uc) {
    (void)uc;
    revo_log("[crash] signal %d at host address %p", sig, si->si_addr);
    dump_native_stack(sig);
    revo_dump_ring();
    _exit(128 + sig);
}
#endif
void revo_dump_ring(void);

static u32 g_entry;
void revo_kernel_set_boot_thread(void);
void revo_platform_run(void);
u32 revo_kernel_setup_main(u32 gp, u32 stack, u32 size);
void revo_kernel_setup_heap(u32 start, u32 size);

#define START_ENTRY 0x00100008u
#define START_GP GAME_VAR(dword_39A6F0)
#define START_STACK 0x01FF0000u
#define START_STACK_SIZE 0x00010000u
#define START_ARGS GAME_VAR(dword_394940)
#define START_MAIN GAME_VAR(func_001002A0)
#define NATIVE_HEAP_SIZE 0x02000000u

static void revo_start(void) {
    revo_kernel_setup_main(START_GP, START_STACK, START_STACK_SIZE);
    revo_kernel_setup_heap(revo_program_end, NATIVE_HEAP_SIZE);
    const s32 code = game_fn_main(START_MAIN, (s32)rd_le32(RAM + START_ARGS), START_ARGS + 4);
    revo_log("[boot] main returned %d", code);
    exit(code);
}

static void guest_body(void *arg) {
    (void)arg;
    revo_kernel_set_boot_thread();
    revo_kernel_init();
    if (g_entry != START_ENTRY) revo_log("[boot] the ELF's entry is 0x%08X, not crt0's start", g_entry);
    revo_kernel_start();
    revo_start();
}

static void *guest_main(void *arg) {
    revo_shadow_stack(64u << 20);
    guest_body(arg);
    return NULL;
}

int main(int argc, char **argv) {
    revo_start_full_program(argc, argv);
#ifdef _WIN32
    AddVectoredExceptionHandler(1, on_native_fault);
    SetUnhandledExceptionFilter(on_crash);
#endif
#ifdef _WIN32
    {
        typedef BOOL(WINAPI * SetContext)(HANDLE);
        SetContext set = (SetContext)(void *)GetProcAddress(GetModuleHandleA("user32.dll"), "SetProcessDpiAwarenessContext");
        if (set) set((HANDLE)-4 );
    }
#endif
    static char last_build[1024];
#ifdef _WIN32
    if (!last_build_assets(last_build, sizeof last_build)) last_build[0] = 0;
#endif
    if (!revo_game_open(argc, argv, last_build)) return 1;
    if (!revo_load_game()) {
        fprintf(stderr, "SLUS_201.38 is not the supported one: Rayman Revolution, PS2, NTSC-U (SLUS-20138)\n");
        return 1;
    }
#ifdef _WIN32
    SetUnhandledExceptionFilter(on_crash);
#else
    {
        struct sigaction sa;
        memset(&sa, 0, sizeof sa);
        sa.sa_sigaction = on_crash;
        sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
        static char altstack[1 << 16];
        stack_t ss = {.ss_sp = altstack, .ss_size = sizeof altstack, .ss_flags = 0};
        sigaltstack(&ss, NULL);
        sigaction(SIGSEGV, &sa, NULL);
        sigaction(SIGBUS, &sa, NULL);
        sigaction(SIGABRT, &sa, NULL);
        sigaction(SIGILL, &sa, NULL);
    }
#endif
    (void)load_elf;
    RAM = revo_native_base();
    u8 *heap = revo_low_alloc(NATIVE_HEAP_SIZE);
    if (!heap) {
        fprintf(stderr, "no memory for the game's heap in its block\n");
        return 1;
    }
    revo_program_end = (u32)(uintptr_t)heap;
    u32 entry = START_ENTRY;

    revo_platform_init();
    g_entry = entry;
    pthread_attr_t at;
    pthread_attr_init(&at);
    pthread_attr_setstacksize(&at, 64u << 20);
    pthread_t guest;
    pthread_create(&guest, &at, guest_main, NULL);
    revo_platform_run();
    return 0;
}
