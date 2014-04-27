prefix ?= /usr/local

tty2out: tty2out.c
	$(CC) -o $@ $^

install: tty2out
	mkdir -p ${prefix}/bin
	install tty2out ${prefix}/bin/

clean:
	rm -rf tty2out

