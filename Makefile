CC := gcc

SRCS := ./ipc.c \
	    ./main.c \
	    ./pipe.c \
		./fifo.c \

OBJS := $(patsubst %.c, %.o, $(SRCS))

all: $(OBJS)
	$(CC) $(OBJS) -T./linker_addon.ld -o out

%.o: %.c
	$(CC) -c -I./ $< -o $@

.PHONY: clean

clean:
	rm $(OBJS)