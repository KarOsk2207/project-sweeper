CC = gcc
CFLAGS = -Wall -Wextra -O2
RAYLIB_PATH = C:/raylib-6.0
INCLUDES = -I$(RAYLIB_PATH)/include
LIBS = -L$(RAYLIB_PATH)/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static

all: sweeper

sweeper: main.c
	$(CC) main.c $(INCLUDES) $(LIBS) $(CFLAGS) -o sweeper.exe
test:
	@echo "No tests yet"
clean:
	rm -f sweeper.exe
