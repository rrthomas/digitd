PREFIX=/usr/local
BINDIR=$(DESTDIR)$(PREFIX)/sbin
MANDIR=$(DESTDIR)$(PREFIX)/man/man8

all:

install:
	cp efingerd.pl $(BINDIR)/efingerd
	mkdir -p $(DESTDIR)/etc/efingerd
	cp examples/* $(DESTDIR)/etc/efingerd

install-doc:
	gzip -9 efingerd.8 -c >$(MANDIR)/efingerd.8.gz
