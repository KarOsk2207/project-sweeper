CC = gcc
CFLAGS = -Wall -Wextra -O2
RAYLIB_PATH = C:/raylib-6.0
INCLUDES = -I$(RAYLIB_PATH)/include
LIBS = -L$(RAYLIB_PATH)/lib -lraylib -lopengl32 -lgdi32 -lwinmm -static

SRC = main.c board.c
OBJ = main.o board.o
TARGET = sweeper.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $(TARGET)

%.o: %.c
	$(CC) -c $(INCLUDES) $(CFLAGS) $< -o $@

# одульные тесты
TEST_SRC = tests/test_board.c board.c tests/unity.c
TEST_TARGET = test_runner.exe

test: $(TEST_SRC)
	$(CC) $(TEST_SRC) -I. -Itests $(CFLAGS) -o $(TEST_TARGET)
	./$(TEST_TARGET)
	rm -f $(TEST_TARGET)

clean:
	rm -f $(OBJ) $(TARGET) $(TEST_TARGET)
