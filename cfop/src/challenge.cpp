#include <cstdio>
#include <cstdint>
#include <coroutine>
#include <unistd.h>

static long g_latch = 0;

static void unlock_gadget(void*) {
    g_latch = 0xC0FFEE;
    puts("  [unlock] *click* latch open");
}
static void reveal_gadget(void*) {
    if (g_latch == 0xC0FFEE) {
        char flag[64]; FILE *f = fopen("flag.txt", "r");
        if (f) {
            fgets(flag, sizeof flag, f);
            printf("  FLAG: %s", flag);
            fclose(f);
        }
    } else {
        puts("  [reveal] vault still sealed");
    }
}

struct Job {
    struct promise_type {
        Job get_return_object() {
            return Job{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend()   noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
    std::coroutine_handle<promise_type> h;
};

static Job real_job() {                   // a normal, benign coroutine
    puts("  [job] doing legitimate work...");
    co_await std::suspend_always{};
    puts("  [job] ...resumed and finished");
}

static void scheduler_resume(std::coroutine_handle<> h) {
    asm volatile("" :: "r"(&h) : "memory");
    h.resume();                           // <-- loads frame[0], calls it (no CFI)
}

alignas(16) static uint64_t g_pool[256];

int main() {
    setvbuf(stdout, nullptr, _IONBF, 0);

    Job j = real_job();
    uint64_t *frame = (uint64_t*)j.h.address();
    printf("[i] a real coroutine frame @ %p\n", (void*)frame);
    printf("[i]   frame[0] (resume ptr)  = %p\n", (void*)frame[0]);
    printf("[i]   frame[1] (destroy ptr) = %p\n", (void*)frame[1]);
    printf("[i] g_pool         = %p   (forge fake frames here)\n", (void*)g_pool);
    printf("[i] unlock_gadget  = %p\n", (void*)unlock_gadget);
    printf("[i] reveal_gadget  = %p\n", (void*)reveal_gadget);
    printf("[i] a forged frame is 16 bytes: [ resume ptr ][ destroy ptr ].\n\n");
    j.h.destroy();

    unsigned char n = 0;
    printf("How many jobs to schedule? ");
    if (read(0, &n, 1) != 1) return 0;
    if (n > 16) n = 16;

    printf("Send %u job frames (%u bytes: 16 each):\n", n, n * 16u);
    read(0, g_pool, (size_t)n * 16);

    for (unsigned i = 0; i < n; i++) {
        void *frame_addr = (void*)(g_pool + i * 2);     // 2 * 8 bytes = 16
        printf("[schedule %u] frame=%p resume_ptr=%p ->\n",
               i, frame_addr, (void*)g_pool[i * 2]);
        fflush(stdout);
        auto handle = std::coroutine_handle<>::from_address(frame_addr);
        scheduler_resume(handle);                       // <-- CFOP fires here
    }
    return 0;
}
