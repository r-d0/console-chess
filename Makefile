CC = gcc

CFLAGS = -Wall -Werror -Iinclude 
LDFLAGS = -lncurses
TARGET = exec
BUILDDIR = build

OBJS = $(BUILDDIR)/main.o \
	   $(BUILDDIR)/board.o \
	   $(BUILDDIR)/pieces.o \
	   $(BUILDDIR)/render.o \

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: src/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/main.o: main.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

.PHONY: clean
