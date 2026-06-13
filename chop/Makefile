CXX := clang++
CFLAGS := -std=c++17 -O1 -g -flto -fvisibility=hidden \
         -fsanitize=cfi -fno-sanitize-trap=cfi \
         -fno-stack-protector -no-pie -fno-pie \
         -Wall -Wno-unused -Wno-invalid-offsetof

BIN      := challenge

all: $(BIN) flag.txt

$(BIN): src/challenge.cpp
	$(CXX) $(CFLAGS) $< -o $@

flag.txt:
	@echo 'ctf{ch0p_picks_the_catch_handler_past_cfi}' > flag.txt

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN) flag.txt

.PHONY: all run clean
