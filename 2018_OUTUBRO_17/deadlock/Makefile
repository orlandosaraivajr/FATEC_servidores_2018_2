INCLUDE_DIRS = 
LIB_DIRS = 

CDEFS=
CFLAGS= -O -g $(INCLUDE_DIRS) $(CDEFS) -DLINUX
LIBS=-lpthread

PRODUCT1=enigma3
PRODUCT2=prgcoding

HFILES=
CFILES1= pthread.c
CFILES2= deadlock.c

SRCS1= ${HFILES} ${CFILES1}
SRCS2= ${HFILES} ${CFILES2}
OBJS1= ${CFILES1:.c=.o}
OBJS2= ${CFILES2:.c=.o}

all: pthread deadlock

clean:
	-rm -f *.o *.d *.exe *~ pthread deadlock

pthread: pthread.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS1) $(LIBS)

deadlock: deadlock.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS2) $(LIBS)

depend:

.c.o:
	$(CC) -MD $(CFLAGS) -c $<

