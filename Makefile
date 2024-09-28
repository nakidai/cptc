OBJS += cptc.o
OBJS += main.o
OBJS += requestHandler.o
OBJS += downloadAvatar.o
OBJS += root.o

CFLAGS += $(shell curl-config --cflags)
CFLAGS += -std=c11

LDLIBS += $(shell curl-config --libs)
LDLIBS += -lcpetpet
LDLIBS += -lcurl
RM = rm -f

all: cptc

utils/%:
	make -C utils

root.c: utils/convert README
	echo "const char *CPTC_root = \"$$(cat README | utils/convert)\";" > $@

cptc: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

clean:
	$(RM) cptc *.o
	make -C utils clean

cptc: $(OBJS)

.PHONY: all clean
