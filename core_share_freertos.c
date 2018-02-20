
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <core_share.h>


/*  Share Buffer.
 *
 *  The share buffer goes in the .core_share section on the ARM/FreeRTOS Target.
 *  For the Linux side, the address of this is placed in a well known place, and
 *  and linux attaches to it with mmap().
 * 
 */
static char __attribute__((__section__(".core_share"))) buf[ SHARE_ALLOC_MEM_SIZE ];


 /* The following are provided by the linker script.
  */
extern unsigned char * __core_share_start; /*!< Share buffer address */
extern unsigned char * __core_sharesz; /*!< Trace buffer size (bytes) */

#define SHARE_ADDR ((uintptr_t)&__core_share_start)
#define SHARE_SIZE ((uintptr_t)&__core_sharesz)

static void traceLogInit();

void *coreShareGetMemoryBlock( int offset, int size )
{
    return( (void *)&buf[ offset ] );
}

int coreShareInit( SHARE_MEM_TYPES type )
{
int i;
volatile uintptr_t *ptr;

printf( "coreShareInit(): Base Address= 0x%08X, type = %d\n", (int)SHARE_ADDR, type );

    ptr = (uintptr_t *)SHARE_ADDR;
    for( i = 0; i < ((uintptr_t)SHARE_SIZE / sizeof(uintptr_t)); i++ ) {
        *ptr++ = 0;
    }

    /* Core 1, FreeRTOS Side initializes this, Core 0, Linux is Client.
     */
    traceLogInit();
    coreShareInitMsgQueues( type );

}

volatile static TRACE_LOG_DS *traceBufferPtr = (TRACE_LOG_DS *)NULL;

static void  traceLogInit()
{
int i;

    traceBufferPtr = (TRACE_LOG_DS *)coreShareGetMemoryBlock( SHARE_TRACE_BUFFER_OFFSET, sizeof(TRACE_LOG_DS) );

    for( i = 0; i < NUM_TRACE_BUFFERS; i++ ) {
	traceBufferPtr->traceLogEntryPtr[ i ] = &traceBufferPtr->traceLog[ i ];
    }

    traceBufferPtr->head = 0;
    traceBufferPtr->tail = 0;

}

static inline void makeTraceEntry( uint32_t programCounter, uint32_t userParam )
{
volatile TRACE_LOG_ENTRY_DS  *ptr;
volatile int head;

    head = traceBufferPtr->head;
    ptr = traceBufferPtr->traceLogEntryPtr[ head++ ];

printf( "makeTraceEntry(): addr == 0x%08X, head = 0x%08X\n", (int)ptr, head );

    read_global_tmr( &ptr->upperTimeStamp, &ptr->lowerTimeStamp );
    ptr->programCounter = programCounter;
    ptr->userParam = userParam;

    if( head == NUM_TRACE_BUFFERS ) {
	traceBufferPtr->head = 0;
    } else {
	traceBufferPtr->head = head;
    }

}

void traceLog( uint32_t programCounter, uint32_t userParam )
{
    makeTraceEntry( programCounter, userParam );
}

void traceLogExit( uint32_t programCounter, uint32_t userParam )
{
    makeTraceEntry( programCounter, (userParam | 0x80000000) );
}

