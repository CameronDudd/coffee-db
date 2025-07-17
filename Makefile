C =gcc 
CFLAGS = -Wall -Wextra -g -std=c99
LIBS = -lsqlite3

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, out/%.o, $(SRC))
TARGET = coffee_diary

all: $(TARGET)

$(TARGET): $(OBJ)
	$(C) -o $@ $^ $(LIBS)

out/%.o: src/%.c | out
	$(C) $(CFLAGS) -c $< -o $@

out:
	mkdir -p out

clean:
	rm -rf out $(TARGET)
