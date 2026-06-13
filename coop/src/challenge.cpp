#include <cstdio>
#include <cstdint>
#include <unistd.h>

static long g_latch = 0;

struct Greeter {
    virtual void hello() { puts("[Greeter] hi"); }
};
struct Polite : Greeter {                // the normal, expected subclass
    void hello() override { puts("[Polite] nice to meet you!"); }
};
struct Unlock : Greeter {
    void hello() override { g_latch = 0xC0FFEE; puts("[Unlock] *click* latch open"); }
};
struct Reveal : Greeter {
    void hello() override {
        if (g_latch != 0xC0FFEE) { puts("[Reveal] still locked"); return; }
        char flag[64]; FILE *f = fopen("flag.txt", "r");
        if (f && fgets(flag, sizeof flag, f))
        {
            printf(" FLAG: %s", flag);
            fclose(f);
        }
    }
};

alignas(16) static unsigned char g_pool[256];

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);

    Unlock u_sample; Reveal r_sample;
    printf("[i] g_pool       = %p   (forge your fake objects here)\n", (void*)g_pool);
    printf("[i] Unlock vtable= %p\n", *(void**)&u_sample);
    printf("[i] Reveal vtable= %p\n", *(void**)&r_sample);
    printf("[i] each fake object is 8 bytes: just a vtable pointer.\n\n");

    unsigned char count = 0;
    printf("How many widgets to render? ");
    if (read(0, &count, 1) != 1) return 0;
    if (count > 32) count = 32;

    printf("Send %u fake objects (%u bytes):\n", count, count * 8);
    read(0, g_pool, (size_t)count * 8);

    for (unsigned i = 0; i < count; i++) {
        Greeter *obj = reinterpret_cast<Greeter *>(g_pool + i * 8);
        printf("[render %u] vptr=%p -> ", i, *(void**)obj);
        fflush(stdout);
        obj->hello();
    }
    return 0;
}
