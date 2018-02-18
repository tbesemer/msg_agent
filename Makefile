
X86_CFLAGS := -I./ -D__X86_TESTING__

.PHONY: x86_test
x86_test:  x86_obj/core_share.o Makefile
	gcc -o x86_obj/x86_test x86_obj/core_share.o
#	gcc -Wl,-T -Wl,x86_lscript.ld -o x86_obj/x86_test x86_obj/core_share.o


x86_obj/core_share.o: core_share.c core_share.h Makefile
	gcc -c -o x86_obj/core_share.o core_share.c ${X86_CFLAGS}



clean:
	rm -f x86_obj/*
