BINDIR=/usr/local/sbin
MANDIR=/usr/local/man/man8

all: efingerd

clean:
	rm -f *~ *.o efingerd

install: efingerd 
	cp efingerd $(BINDIR)
	mkdir -p $(DESTDIR)/etc/efingerd
	cp examples/* $(DESTDIR)/etc/efingerd

install-doc: efingerd 
	gzip -9 efingerd.8 -c >$(MANDIR)/efingerd.8.gz
