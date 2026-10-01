CC       = clang
CXX      = clang++
CFLAGS   = -std=c17 -Wall -Wextra -g
CXXFLAGS = -std=c++20 -Wall -Wextra -g -Iinclude
SAN      = -fsanitize=address,undefined

# ---- Day 1: C parser (src_c/) ----
parser: src_c/parser.c
	$(CC) $(CFLAGS) $(SAN) src_c/parser.c -o parser

run: parser
	./parser jobs.txt

# macOS ASan can't detect leaks, so use Apple's `leaks` tool on a plain build
leakcheck: src_c/parser.c
	$(CC) $(CFLAGS) src_c/parser.c -o parser_plain
	leaks --atExit -- ./parser_plain jobs.txt

# ---- C++ scheduler: every .cpp in src/, headers in include/ ----
SRCS    = $(wildcard src/*.cpp)
HEADERS = $(wildcard include/minisched/*.hpp)

minisched: $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SAN) $(SRCS) -o minisched

runcpp: minisched
	./minisched jobs.txt

# ThreadSanitizer build (detects data races). Can't be combined with ASan.
tsan: $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -fsanitize=thread $(SRCS) -o minisched_tsan
	./minisched_tsan jobs.txt

clean:
	rm -rf parser parser_plain minisched minisched_tsan *.dSYM

.PHONY: run leakcheck runcpp tsan clean
