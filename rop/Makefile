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
	@echo 'ctf{r0p_w4lks_r1ght_p4st_f0rw4rd_edge_cfi}' > flag.txt

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN) flag.txt

.PHONY: all run clean
