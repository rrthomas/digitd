
BINDIR=/usr/local/sbin
MANDIR=/usr/local/man/man8

CC = gcc 
CFLAGS = -O2 -Wall -Wsurprising

#or, if you do not have libinent at all:
#CFLAGS = -O2 -Wall -DDONT_HAVE_LIBIDENT

#uncomment the following line if you do have libident.so
#LDFLAGS = -lident


all: efingerd

efingerd: efingerd.o child.o
#	$(CC) $(CFLAGS) $(LDFLAGS) efingerd.o child.o -o efingerd
#replace the line below with the line above if you have libident.so, or 
#do not have libident at all
#modify path to libident.a if necessary
	$(CC) $(CFLAGS) $(LDFLAGS) efingerd.o child.o /usr/lib/libident.a -o efingerd
	strip efingerd

efingerd.o: efingerd.c
	$(CC) $(CFLAGS) -c efingerd.c

child.o: child.c
	$(CC) $(CFLAGS) -c child.c

clean:
	rm -f *~ *.o efingerd

install: efingerd 
	cp efingerd $(BINDIR)
	mkdir -p $(DESTDIR)/etc/efingerd
	cp examples-standard/* $(DESTDIR)/etc/efingerd

install-doc: efingerd 
	gzip -9 efingerd.8 -c >$(MANDIR)/efingerd.8.gz
