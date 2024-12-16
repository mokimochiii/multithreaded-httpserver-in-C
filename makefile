EXEC = server
SRCS = $(wildcard *.c)
HDRS = $(wildcard *.h)
OBJS = $(SRCS:%.c=%.o)
FORMATS  = $(SRCS:%.c=.format/%.c.fmt) $(HDRS:%.h=.format/%.h.fmt)

CC = clang
FORMAT = clang-format
CFLAGS = -Wall -Wpedantic -Wextra -Werror -DDEBUG

.PHONY: all clean format

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -o $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(EXEC) $(OBJS)

nuke: clean
	rm -rf .format

format: $(FORMATS)

.format/%.c.fmt: %.c
	mkdir -p .format
	$(FORMAT) -i $<
	touch $@

.format/%.h.fmt: %.h
	mkdir -p .format
	$(FORMAT) -i $<
	touch $@

