OBJS += cptc.o
OBJS += main.o
OBJS += requestHandler.o

CFLAGS += -std=c11
RM = rm -f

all: cptc

cptc: $(OBJS)

clean:
	$(RM) cptc *.o

cptc: $(OBJS)
