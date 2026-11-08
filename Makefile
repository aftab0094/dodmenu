CC ?= gcc
PKGCONFIG = $(shell which pkg-config)
CFLAGS = $(shell $(PKGCONFIG) --cflags gtk4 gtk4-layer-shell-0)
LIBS = $(shell $(PKGCONFIG) --libs gtk4 gtk4-layer-shell-0)
default:
	$(CC) -o main main.c $(LIBS) $(CFLAGS)
clean:
	rm -f main
