CFLAGS += -Wall

BINDIR=/usr/bin
MANDIR=/usr/share/man/man8

uefivar: uefivar.o
	$(CC) $< -lm -o $@

uefivar.8.gz: uefivar.8
	gzip -c $< > $@

clean:
	rm -f uefivar uefivar.o uefivar.8.gz

install: uefivar uefivar.8.gz
	mkdir -p ${DESTDIR}${BINDIR}
	cp uefivar ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MANDIR}
	cp uefivar.8.gz ${DESTDIR}${MANDIR}
