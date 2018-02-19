
X86_CFLAGS := -I./ -D__X86_TESTING__
FREERTOS_CFLAGS := -I./ -D__FREERTOS__

FREERTOS_CC := arm-xilinx-eabi-gcc

all: freertos_obj/core_share.o freertos_obj/core_share_linux.o

.PHONY: x86_test
x86_test:  x86_obj/core_share.o x86_obj/core_share_linux.o Makefile
	gcc -o x86_obj/x86_test x86_obj/core_share.o

.PHONY: free_rtos_obj/core_share.o
freertos_obj/core_share.o: core_share.c core_share.h Makefile
	${FREERTOS_CC} -c -o freertos_obj/core_share.o core_share.c ${FREERTOS_CFLAGS}

.PHONY: free_rtos_obj/core_share_freertos.o
freertos_obj/core_share_linux.o: core_share_freertos.c core_share.h Makefile
	${FREERTOS_CC} -c -o freertos_obj/core_share_freertos.o core_share_freertos.c ${FREERTOS_CFLAGS}

.PHONY: x86_obj/core_share.o
x86_obj/core_share.o: core_share.c core_share.h Makefile
	gcc -c -o x86_obj/core_share.o core_share.c ${X86_CFLAGS}

.PHONY: x86_obj/core_share_linux.o
x86_obj/core_share_linux.o: core_share_linux.c core_share.h Makefile
	gcc -c -o x86_obj/core_share_linux.o core_share_linux.c ${X86_CFLAGS}

clean:
	rm -f x86_obj/* freertos_obj/*
