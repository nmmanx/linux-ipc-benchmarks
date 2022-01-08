TARGET := ipcbm
CC := gcc

SRCS := ./ipc.c \
	    ./main.c \
	    ./pipe.c \
		./fifo.c \

OBJS := $(patsubst %.c, %.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -T./linker_addon.ld -o $@

%.o: %.c
	$(CC) -c -I./ $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJS) $(TARGET)