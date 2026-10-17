CC ?= cc
CPPFLAGS ?= -Iinclude
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?=

TARGET := build/c-malloc-context
SOURCES := $(wildcard src/*.c)
OBJECTS := $(patsubst src/%.c,build/%.o,$(SOURCES))
DEPFILES := $(OBJECTS:.o=.d)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

build/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPFILES)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build
