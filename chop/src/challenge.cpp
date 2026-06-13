#include <cstdio>
#include <cstring>
#include <cstddef>
#include <typeinfo>
#include <unistd.h>

static long g_latch = 0;

struct BenignError {};
struct UnlockError {};
struct RevealError {};

extern "C" {
    void* __cxa_allocate_exception(unsigned long);
    void  __cxa_throw(void*, std::type_info*, void(*)(void*));
}

struct Request {
    char             buf[24];
    std::type_info  *error_ti;
};

static void process(Request *r) {
    if (r->buf[0] == 'O' && r->buf[1] == 'K') { puts("  [process] request OK"); return; }
    puts("  [process] invalid -> throwing error (runtime selects the catch)...");
    void *exc = __cxa_allocate_exception(8);
    __cxa_throw(exc, r->error_ti, nullptr);
}

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);

    printf("[i] ti(BenignError) = %p\n", (void*)&typeid(BenignError));
    printf("[i] ti(UnlockError) = %p   <- catch flips the latch\n", (void*)&typeid(UnlockError));
    printf("[i] ti(RevealError) = %p   <- catch prints the flag\n", (void*)&typeid(RevealError));
    printf("[i] sizeof(Request)=%zu, error_ti at offset %zu (right after buf[24]).\n\n",
           sizeof(Request), offsetof(Request, error_ti));

    unsigned char n = 0;
    printf("How many requests? ");
    if (read(0, &n, 1) != 1) return 0;
    if (n > 16) n = 16;

    Request q[16];
    printf("Send %u request records (%zu bytes each: buf[24] + 8-byte error_ti):\n",
           n, sizeof(Request));
    for (unsigned i = 0; i < n; i++)
        read(0, &q[i], sizeof(Request));

    for (unsigned i = 0; i < n; i++) {
        printf("[dispatch %u]\n", i);
        try {
            process(&q[i]);
        }
        catch (BenignError&) {
            puts("  [catch BenignError] logged. nothing to see here.");
        }
        catch (UnlockError&) {
            g_latch = 0xC0FFEE;
            puts("  [catch UnlockError] *click* latch open");
        }
        catch (RevealError&) {
            if (g_latch == 0xC0FFEE) {
                char flag[64]; FILE *f = fopen("flag.txt", "r");
                if (f && fgets(flag, sizeof flag, f)) printf("  FLAG: %s", flag);
                else puts("  FLAG: ctf{ch0p_picks_the_catch_handler_past_cfi}");
                if (f) fclose(f);
            } else {
                puts("  [catch RevealError] vault still sealed");
            }
        }
    }
    return 0;
}
