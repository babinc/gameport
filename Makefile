CC      = cc
CFLAGS  = -std=c99 -Wall -Wextra -pedantic -O2
LDFLAGS = -lm
SRC     = src/main.c src/term.c src/ui.c src/catalog.c src/install.c src/util.c
GAMESRC = $(wildcard src/games/*.c)
OBJ     = $(SRC:.c=.o) $(GAMESRC:.c=.o)
BIN     = gameport

PREFIX ?= /usr/local

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

src/games/%.o: src/games/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

debug:
	$(CC) -std=c99 -Wall -Wextra -pedantic -g -O0 -o $(BIN) $(SRC) $(GAMESRC) $(LDFLAGS)

clean:
	rm -f $(OBJ) $(BIN)

install: $(BIN)
	install -d $(PREFIX)/bin
	install -m 755 $(BIN) $(PREFIX)/bin/

uninstall:
	rm -f $(PREFIX)/bin/$(BIN)

.PHONY: all clean debug install uninstall
