#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

static char secret_flag[64];

void print_flag_fnptr(uint64_t key)
{
    if (key == 0xD09D09D09D09D09DUL)
        printf(" FLAG: %s\n", secret_flag);
    else
        puts("[print_flag_fnptr] reached, but this should never run under CFI.");
    fflush(stdout);
    _exit(0);
}

typedef void (*renderer_t)(const char *label);

static void render_plain(const char *label) { printf("[vm] label = \"%s\"\n", label); }
static void render_loud (const char *label) { printf("[VM] LABEL = \"%s\"!!\n", label); }

static renderer_t renderers[2] = { render_plain, render_loud };

struct vm {
    char       label[32];
    uint64_t  *ptr;
    renderer_t render;
    uint64_t   acc;
    uint64_t   cells[8];
};

static void op_name(struct vm *vm)
{
    printf("[name] send raw bytes for the label (overflows past 32):\n> ");
    fflush(stdout);
    read(0, vm->label, 256);          /* 256 bytes into a 32-byte buffer */
}

static void op_load(struct vm *vm, long i)
{
    if (i < 0 || i > 7) { puts("[load] cell out of range (0..7)"); return; }
    vm->ptr = &vm->cells[i];
    printf("[load] ptr -> cells[%ld]\n", i);
}

static void op_peek(struct vm *vm)
{
    printf("[peek] *ptr = 0x%016lx\n", *vm->ptr);
}

static void op_emit(struct vm *vm)
{
    uint64_t word = *vm->ptr;
    fwrite(&word, 1, sizeof word, stdout);
    fflush(stdout);
}

static void op_next(struct vm *vm)
{
    vm->ptr++;
}

static void op_render(struct vm *vm)
{
    puts("[render] calling vm.render(label) -- this is an indirect call:");
    fflush(stdout);
    vm->render(vm->label);            /* <-- clang CFI checks target type */
}

static void print_help(void)
{
    puts("opcodes:");
    puts("  name        read raw bytes into the label buffer");
    puts("  load <i>    point ptr at cells[i]   (i = 0..7)");
    puts("  peek        print *ptr as a 64-bit hex word");
    puts("  emit        write the 8 bytes at *ptr to stdout");
    puts("  next        advance ptr by one 64-bit word (ptr++)");
    puts("  render      render the label via vm.render(label)");
    puts("  help        show this list again");
    puts("  quit        leave the VM");
}

static void vm_run(void)
{
    struct vm vm;
    memset(&vm, 0, sizeof vm);
    vm.ptr    = &vm.cells[0];         /* safe default: points inside cells */
    vm.render = renderers[0];         /* safe default: a type-correct cb   */

    char line[64];
    for (;;) {
        printf("\nvm> ");
        fflush(stdout);
        if (!fgets(line, sizeof line, stdin)) break;

        if (!strncmp(line, "name", 4))        op_name(&vm);
        else if (!strncmp(line, "load", 4))   op_load(&vm, strtol(line + 4, NULL, 0));
        else if (!strncmp(line, "peek", 4))   op_peek(&vm);
        else if (!strncmp(line, "emit", 4))   op_emit(&vm);
        else if (!strncmp(line, "next", 4))   op_next(&vm);
        else if (!strncmp(line, "render", 6)) op_render(&vm);
        else if (!strncmp(line, "help", 4))   print_help();
        else if (!strncmp(line, "quit", 4))   break;
        else { puts("unknown opcode."); print_help(); }
    }
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    FILE *f = fopen("flag.txt", "r");
    if (!f)
	    return 1;
    if (f)
    {
	    fgets(secret_flag, sizeof secret_flag, f);
	    fclose(f);
	}

    printf("[i] secret_flag is at %p\n", (void *)secret_flag);
    printf("[i] (ignore me) print_flag_fnptr is at %p\n", (void *)print_flag_fnptr);

    puts("\n--- CFI vs DOP demo ---");
    puts("Intended use: 'load <i>' to point at a cell, then 'peek'/'emit'.");
    puts("The VM should only ever touch its own cells[8]...\n");
    print_help();

    vm_run();
    return 0;
}
