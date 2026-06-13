#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_flag(unsigned long key)
{
    if (key != 0xC0FFEE1234BEEF99UL) {
        printf("[print_flag] wrong key 0x%lx -- no flag for you.\n", key);
        return;
    }

    puts("\n=================================================");
    puts(" CFI was ENABLED... and you still got here. ");
    char buf[64];
    FILE *f = fopen("flag.txt", "r");
    if (f && fgets(buf, sizeof buf, f)) {
        printf(" FLAG: %s\n", buf);
        fclose(f);
    }
    puts("=================================================\n");
    fflush(stdout);
    _exit(0);
}

typedef void (*handler_t)(void);   /* takes nothing, returns nothing */

static void say_hello(void) { puts("[handler] hello from say_hello()"); }
static void say_bye(void)   { puts("[handler] bye from say_bye()");     }

static handler_t handlers[2] = { say_hello, say_bye };

static void forward_edge_demo(void)
{
    handler_t fp = handlers[0];

    puts("\n[mode 1] Forward-edge (indirect call) attack");
    puts("We will call a handler through a function pointer.");
    fflush(stdout);

    puts("[*] Repointing the handler at print_flag() (wrong type!)...");
    fp = (handler_t)(void *)print_flag;      /* type-confused pointer  */

    puts("[*] Performing the indirect call now:");
    fflush(stdout);
    fp();
    puts("[*] (If you see this line, CFI did NOT stop the call.)");
}

static void backward_edge_demo(void)
{
    char buf[64];

    puts("\n[mode 2] Backward-edge (return address) attack");
    puts("Send me your input. read() has no idea how big buf is.");
    printf("> ");
    fflush(stdout);

    read(0, buf, 512);

    printf("[*] You said: %s\n", buf);
    puts("[*] Returning now (where to?) ...");
    fflush(stdout);
}

static void menu(void)
{
    puts("\n--- CFI vs ROP demo ---");
    puts(" 1) forward-edge attack  (CFI should block this)");
    puts(" 2) backward-edge attack (CFI cannot see this)");
    puts(" q) quit");
    printf("choice> ");
    fflush(stdout);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("[i] print_flag is at %p (you may need this)\n",
           (void *)print_flag);

    char line[16];
    for (;;) {
        menu();
        if (!fgets(line, sizeof line, stdin)) break;
        switch (line[0]) {
            case '1': forward_edge_demo();  break;
            case '2': backward_edge_demo(); return 0;
            case 'q': return 0;
            default:  puts("?"); break;
        }
    }
    return 0;
}
