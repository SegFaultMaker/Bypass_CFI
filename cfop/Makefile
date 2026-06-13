CXX := clang++
CFLAGS := -std=c++20 -O1 -g -flto -fvisibility=hidden \
         -fsanitize=cfi -fno-sanitize-trap=cfi \
         -fno-stack-protector -no-pie -fno-pie \
         -Wall -Wno-unused

BIN      := challenge

all: $(BIN) flag.txt

$(BIN): src/challenge.cpp
	$(CXX) $(CFLAGS) $< -o $@

flag.txt:
	@echo 'ctf{cfop_resumes_a_forged_coroutine_frame}' > flag.txt

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN) flag.txt

.PHONY: all run clean
