#ifndef __CORE_SHARE__
#define __CORE_SHARE__

#ifdef  __X86_TESTING__
#define uintptr_t long long
#else
#define uintptr_t uint32_t
#endif

extern void coreShareInit();

#endif
