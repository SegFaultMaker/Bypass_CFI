CC      := clang
CFLAGS  := -O1 -g -flto -fvisibility=hidden \
           -fsanitize=cfi -fno-sanitize-trap=cfi \
           -fno-stack-protector -no-pie -fno-pie \
           -Wall -Wno-unused

BIN      := challenge

all: $(BIN) flag.txt

$(BIN): src/challenge.c
	$(CC) $(CFLAGS) $< -o $@

flag.txt:
	@echo 'ctf{d4t4_0r13nt3d_pwn_n33ds_n0_c0d3_p01nt3rs}' > flag.txt

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN) flag.txt

.PHONY: all run clean
