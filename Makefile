.PHONY: clean

OBJS = md5.o md_common.c reg.o sb.o snak.o snakext.o
CFLAGS += -std=c11 -D_POSIX_C_SOURCE=2

all: sb

md5.o: md5.c md5.h
md_common.o: md_common.c md5.h
reg.o: h.h md5.h
sb.o: h.h
snak.o: snak.c h.h
snakext.o: snakext.c h.h md5.h

sb: $(OBJS)
	$(CC) -o sb $(OBJS)

clean:
	rm -f sb *.o

dist:
	mkdir sb-1.2
	cp *.c *.h README Makefile sb-1.2
	cp -r doc/ sb-1.2
	groff -Tpdf -mdoc doc/sb.1 > sb-1.2/doc/sb.1.pdf
	groff -Tpdf -ms doc/drm.ms > sb-1.2/doc/drm.ms.pdf
	tar czf sb-1.2.tar.gz --owner=root --group=root --format=ustar sb-1.2
	rm -rf sb-1.2

