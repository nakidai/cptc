OBJS += cptc.o
OBJS += main.o
OBJS += requestHandler.o
OBJS += downloadAvatar.o

CFLAGS += $(shell curl-config --cflags)
CFLAGS += -std=c11

LDFLAGS += $(shell curl-config --libs)
LDFLAGS += -lcpetpet
RM = rm -f

all: cptc

cptc: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) cptc *.o

cptc: $(OBJS)
