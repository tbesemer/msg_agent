#ifndef __CORE_SHARE__
#define __CORE_SHARE__

#ifdef  __X86_TESTING__
#define uintptr_t long long
#else
#define uintptr_t uint32_t
#endif

extern int coreShareInit( int type );
extern void *coreShareGetMemoryBlock( int offset, int size );

/*  On FreeRTOS, SIZE/ADDR this must mache what is in <lscript.ld>
 *  MASK is used on Linux for mmap() operations.
 */
#define SHARE_ALLOC_MEM_SIZE    0xC00000
#define SHARE_ALLOC_MEM_ADDR    0xC00000
#define SHARE_ALLOC_MEM_MASK	(SHARE_ALLOC_MEM_SIZE - 1)

/*  Offsets in Shared Memory for key structures.
 */
#define SHARE_TRACE_BUFFER_OFFSET     0x00000000
#define SHARE_MSG_SYSTEM_OFFSET       0x00020000


#define NUM_TRACE_BUFFERS       512

typedef struct {

    volatile uint32_t   upperTimeStamp;
    volatile uint32_t   lowerTimeStamp;
    volatile uint32_t   programCounter;
    volatile uint32_t   userParam;

} TRACE_LOG_ENTRY_DS;

typedef struct {

    volatile int                 head;
    volatile int                 tail;
    volatile TRACE_LOG_ENTRY_DS  *traceLogEntryPtr[ NUM_TRACE_BUFFERS ];
    volatile TRACE_LOG_ENTRY_DS  traceLog[ NUM_TRACE_BUFFERS ];

} TRACE_LOG_DS;



#endif
