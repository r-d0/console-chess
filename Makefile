CC = gcc

CFLAGS = -Wall -Werror -Iinclude 
TARGET = exec
BUILDDIR = build

OBJS = $(BUILDDIR)/main.o \
	   $(BUILDDIR)/board.o \
	   $(BUILDDIR)/pieces.o \
	   $(BUILDDIR)/render.o \

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) -lncurses

$(BUILDDIR)/%.o: src/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -lncurses

$(BUILDDIR)/main.o: main.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@ -lncurses

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

.PHONY: clean
