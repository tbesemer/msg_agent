
X86_CFLAGS := -I./ -D__X86_TESTING__

all: x86_obj/core_share.o x86_obj/core_share_linux.o

.PHONY: x86_test
x86_test:  x86_obj/core_share.o x86_obj/core_share_linux.o Makefile
	gcc -o x86_obj/x86_test x86_obj/core_share.o


.PHONY: x86_obj/core_share.o
x86_obj/core_share.o: core_share.c core_share.h Makefile
	gcc -c -o x86_obj/core_share.o core_share.c ${X86_CFLAGS}

.PHONY: x86_obj/core_share_linux.o
x86_obj/core_share_linux.o: core_share_linux.c core_share.h Makefile
	gcc -c -o x86_obj/core_share_linux.o core_share_linux.c ${X86_CFLAGS}

clean:
	rm -f x86_obj/*
