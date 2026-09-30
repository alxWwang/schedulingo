CC       = clang
CXX      = clang++
CFLAGS   = -std=c17 -Wall -Wextra -g
CXXFLAGS = -std=c++20 -Wall -Wextra -g
SAN      = -fsanitize=address,undefined

# ---- Day 1: C parser ----
parser: parser.c
	$(CC) $(CFLAGS) $(SAN) parser.c -o parser

run: parser
	./parser jobs.txt

# macOS ASan can't detect leaks, so use Apple's `leaks` tool on a plain build
leakcheck: parser.c
	$(CC) $(CFLAGS) parser.c -o parser_plain
	leaks --atExit -- ./parser_plain jobs.txt

# ---- Day 2: C++ parser ----
minisched: minisched.cpp
	$(CXX) $(CXXFLAGS) $(SAN) minisched.cpp -o minisched

runcpp: minisched
	./minisched jobs.txt

clean:
	rm -rf parser parser_plain minisched *.dSYM

.PHONY: run leakcheck runcpp clean
