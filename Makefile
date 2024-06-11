CFLAGS = -g -O0 -Wall -Wextra
LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm

main: main.c
	gcc $(CFLAGS) -o main main.c glad.c -Iinclude $(LDFLAGS)

.PHONY: clear test debug altfile win

clear:
	rm -f main

test:
	./main

debug:
	gdb --tui main

altfile:
	gcc $(CFLAGS) -o test test.c glad.c -Iinclude $(LDFLAGS)
	./test

WINCFLAGS = -O2
WINLDFLAGS = lib/winLib/libglfw3.a -lgdi32 -lopengl32

win:
	x86_64-w64-mingw32-gcc $(WINCFLAGS) -o mainWinRelease main.c glad.c -Iinclude $(WINLDFLAGS)
	wine ./mainWinRelease
