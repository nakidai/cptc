OBJS += cptc.o
OBJS += downloadAvatar.o
OBJS += main.o
OBJS += requestHandler.o
OBJS += root.o

CFLAGS += $(shell curl-config --cflags)
CFLAGS += -std=c11

LDLIBS += $(shell curl-config --libs)
LDLIBS += -lcurl
LDLIBS += -lcpetpet
RM = rm -f

all: cptc

utils/%:
	make -C utils

root.c: utils/convert README
	echo "const char *CPTC_root = \"$$(cat README | utils/convert)\";" > $@

cptc: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

clean:
	$(RM) cptc root.c *.o
	make -C utils clean

cptc: $(OBJS)

.PHONY: all clean
