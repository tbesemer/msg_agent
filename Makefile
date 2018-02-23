
X86_CFLAGS := -I./ -D__X86_TESTING__
FREERTOS_CFLAGS := -I./ -D__FREERTOS__ -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=softfp
ARM_CFLAGS := -I./ -D__ARM__

FREERTOS_CC := arm-none-eabi-gcc
FREERTOS_AR := arm-none-eabi-ar
FREERTOS_RANLIB := arm-none-eabi-ranlib

ARM_CC := arm-linux-gnueabihf-gcc
ARM_AR := arm-linux-gnueabihf-ar
ARM_RANLIB := arm-linux-gnueabihf-ranlib

all: lib/sharefreertoslib.a lib/sharearmlib.a

.PHONY: lib/sharefreertoslib.a
lib/sharefreertoslib.a: freertos_obj/core_share.o freertos_obj/core_share_freertos.o
	${FREERTOS_AR} cr lib/sharefreertoslib.a freertos_obj/core_share.o freertos_obj/core_share_freertos.o
	${FREERTOS_RANLIB} lib/sharefreertoslib.a

.PHONY: freertos_obj/core_share.o
freertos_obj/core_share.o: core_share.c core_share.h Makefile
	${FREERTOS_CC} -c -o freertos_obj/core_share.o core_share.c ${FREERTOS_CFLAGS}

.PHONY: freertos_obj/core_share_freertos.o
freertos_obj/core_share_freertos.o: core_share_freertos.c core_share.h Makefile
	${FREERTOS_CC} -c -o freertos_obj/core_share_freertos.o core_share_freertos.c ${FREERTOS_CFLAGS}

.PHONY: lib/sharex86lib.a
lib/sharex86lib.a: x86_obj/core_share.o x86_obj/core_share_linux.o
	ar cr lib/sharex86lib.a x86_obj/core_share.o x86_obj/core_share_linux.o
	ranlib lib/sharex86lib.a

.PHONY: x86_obj/core_share.o
x86_obj/core_share.o: core_share.c core_share.h Makefile
	gcc -c -o x86_obj/core_share.o core_share.c ${X86_CFLAGS}

.PHONY: x86_obj/core_share_linux.o
x86_obj/core_share_linux.o: core_share_linux.c core_share.h Makefile
	gcc -c -o x86_obj/core_share_linux.o core_share_linux.c ${X86_CFLAGS}

.PHONY: lib/sharearmlib.a
lib/sharearmlib.a: arm_obj/core_share.o arm_obj/core_share_linux.o
	${ARM_AR} cr lib/sharearmlib.a arm_obj/core_share.o arm_obj/core_share_linux.o
	${ARM_RANLIB} lib/sharearmlib.a

.PHONY: arm_obj/core_share.o
arm_obj/core_share.o: core_share.c core_share.h Makefile
	${ARM_CC} -c -o arm_obj/core_share.o core_share.c ${X86_CFLAGS}

.PHONY: arm_obj/core_share_linux.o
arm_obj/core_share_linux.o: core_share_linux.c core_share.h Makefile
	${ARM_CC} -c -o arm_obj/core_share_linux.o core_share_linux.c ${X86_CFLAGS}

clean:
clean:
	rm -f x86_obj/* arm_obj/* freertos_obj/* lib/*
