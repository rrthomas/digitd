PREFIX=/usr/local
BINDIR=$(DESTDIR)$(PREFIX)/sbin
MANDIR=$(DESTDIR)$(PREFIX)/man/man8
PROGRAM=digitd

all:

install:
	cp $(PROGRAM).pl $(BINDIR)/$(PROGRAM)
	mkdir -p $(DESTDIR)/etc/$(PROGRAM)
	cp scripts/* $(DESTDIR)/etc/$(PROGRAM)

install-doc:
	gzip -9 $(PROGRAM).8 -c >$(MANDIR)/$(PROGRAM).8.gz
