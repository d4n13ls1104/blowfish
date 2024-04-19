CC := gcc

FSANITIZE := -fsanitize=address -fsanitize=undefined -fsanitize=leak
WARNINGS := -Wall -Wextra

CFLAGS := -std=c99 -O2 $(WARNINGS) `sdl2-config --libs --cflags` -lSDL2_image

SRC_DIR := src
INCLUDE_DIR := include
BUILD_DIR := build

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
INCS := -I$(INCLUDE_DIR)

TARGET := out

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

clean:
	rm -r $(BUILD_DIR)/*.o $(TARGET)
