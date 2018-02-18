
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <core_share.h>


/*  Share Buffer.
 *
 *  The share buffer goes in the .core_share section on the ARM/FreeRTOS Target.
 *  For the Linux side, the address of this is placed in a well known place, and
 *  and linux attaches to it with mmap().
 */
#define BUFSZ	0xC0000			/*!< Share buffer size (bytes) */
static char __attribute__((__section__(".core_share"))) buf[BUFSZ];


 /* The following are provided by the linker script.
  */
extern unsigned char * __core_share_start; /*!< Share buffer address */
extern unsigned char * __core_sharesz; /*!< Trace buffer size (bytes) */

#ifdef  __X86_TESTING__
#define SHARE_ADDR ((uintptr_t)buf)
#define SHARE_SIZE ((uintptr_t)BUFSZ)
#else
#define SHARE_ADDR ((uintptr_t)&__core_share_start)
#define SHARE_SIZE ((uintptr_t)&__core_sharesz)
#endif

#define	TRACE_BUFFER_OFFSET	0x00000000
#define	MSG_SYSTEM_OFFSET	0x00020000

void traceLogInit();

uintptr_t *coreShareGetMemoryBlock( uint32_t offset, int size )
{
    return( (void *)&buf[ offset ] );
}

void coreShareInit()
{
int i;
volatile uintptr_t *ptr;

    ptr = (uintptr_t *)SHARE_ADDR;
    for( i = 0; i < ((uintptr_t)BUFSZ / sizeof(uintptr_t)); i++ ) {
        *ptr++ = 0;
    }

    traceLogInit();

}


#define	NUM_TRACE_BUFFERS	512

typedef struct {

    uint32_t   upperTimeStamp;
    uint32_t   lowerTimeStamp;
    uint32_t   programCounter;
    uint32_t   userParam;

} TRACE_LOG_ENTRY_DS;

typedef struct {

    volatile int                 head;
    volatile int                 tail;
    volatile TRACE_LOG_ENTRY_DS  *traceLogEntryPtr[ NUM_TRACE_BUFFERS ];
    volatile TRACE_LOG_ENTRY_DS  traceLog[ NUM_TRACE_BUFFERS ];

} TRACE_LOG_DS;

volatile static TRACE_LOG_DS *traceBufferPtr = (TRACE_LOG_DS *)NULL;

void  traceLogInit()
{
int i;

    traceBufferPtr = (TRACE_LOG_DS *)coreShareGetMemoryBlock( TRACE_BUFFER_OFFSET, sizeof(TRACE_LOG_DS) );

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
    if( head == NUM_TRACE_BUFFERS ) {
	traceBufferPtr->head = 0;
    } else {
	traceBufferPtr->head = head;
    }

#ifndef  __X86_TESTING__
    read_global_tmr( &ptr->upperTimeStamp, &ptr->lowerTimeStamp );
#endif
    ptr->programCounter = programCounter;
    ptr->userParam = userParam;
}

void traceLog( uint32_t programCounter, uint32_t userParam )
{
    makeTraceEntry( programCounter, userParam );
}

void traceLogExit( uint32_t programCounter, uint32_t userParam )
{
    makeTraceEntry( programCounter, (userParam | 0x80000000) );
}


#define SHARED_MSG_QUEUE_COUNT	512
#define SHARED_MSG_QUEUE_SIZE	512

typedef struct {

    volatile int     head;
    volatile int     tail;
    volatile int     count;
    volatile int     size;
    volatile uint8_t *mcbBase;    /*  Message Control Buffers Base Address    */

} SHARED_MSG_QUEUE_DS;

typedef struct {

    volatile SHARED_MSG_QUEUE_DS rxQueueCtl;    /* RX Queue, Client Side       */
    volatile SHARED_MSG_QUEUE_DS txQueueCtl;    /* TX Queue, Client Side       */

    volatile uint8_t  rxQueueAlloc[ (SHARED_MSG_QUEUE_SIZE * SHARED_MSG_QUEUE_COUNT) ];
    volatile uint8_t  txQueueAlloc[ (SHARED_MSG_QUEUE_SIZE * SHARED_MSG_QUEUE_COUNT) ];

} SHARED_MSG_QUEUE_MGR_DS;

volatile static SHARED_MSG_QUEUE_MGR_DS *queueCtlManagerPtr = (SHARED_MSG_QUEUE_MGR_DS *)NULL;

void coreShareInitMsgQueues( SHARED_MSG_QUEUE_MGR_DS *qCtlMgrPtr )
{

    queueCtlManagerPtr = (SHARED_MSG_QUEUE_MGR_DS *)coreShareGetMemoryBlock( MSG_SYSTEM_OFFSET,
						sizeof(SHARED_MSG_QUEUE_MGR_DS) );

    qCtlMgrPtr->rxQueueCtl.head = 0;
    qCtlMgrPtr->rxQueueCtl.tail = 0;
    qCtlMgrPtr->rxQueueCtl.count = SHARED_MSG_QUEUE_COUNT;
    qCtlMgrPtr->rxQueueCtl.size = SHARED_MSG_QUEUE_SIZE;
    qCtlMgrPtr->rxQueueCtl.mcbBase = qCtlMgrPtr->rxQueueAlloc;

    qCtlMgrPtr->txQueueCtl.head = 0;
    qCtlMgrPtr->txQueueCtl.tail = 0;
    qCtlMgrPtr->txQueueCtl.count = SHARED_MSG_QUEUE_COUNT;
    qCtlMgrPtr->txQueueCtl.size = SHARED_MSG_QUEUE_SIZE;
    qCtlMgrPtr->txQueueCtl.mcbBase = qCtlMgrPtr->txQueueAlloc;

}

static int coreShareWriteQueue( SHARED_MSG_QUEUE_DS *msgQPtr, uint8_t *msgPtr, int size )
{
         int     tHead;
volatile uint8_t *dstPtr;

    /*  We trust that somebody upstream made sure it will fit, but just to avoid
     *  corruption, ensure we fit.
     */
    if( size > msgQPtr->size ) {
        return( -1 );
    }

    /*  Form and bump our head to next slot, but current slot is where we store data
     *  if we have space.
     */
    tHead = (msgQPtr->head + 1);
    if( tHead == msgQPtr->count ) {
	tHead = 0;
    }

    /*  Check to see if the Queue is full, simply return if we can't service
     *  the request - callers responsibilty to retry.
     */
    if( tHead == msgQPtr->tail ) {
        return( -2 );
    }

    /*  Space, copy data.  The Queue Buffer are msgQPtr->size blocks starting
     *  at msgQPtr->mcbBase, so a physical addressis formed based on entry index,
     *  with that index being what 'head' currently points to.
     */
    dstPtr = &msgQPtr->mcbBase[ msgQPtr->head * msgQPtr->size ];
    memcpy( (void *)dstPtr, (void *)msgPtr, size );

    /*  Push the new index out, letting readers know new data is available.
     */
    msgQPtr->head = tHead;

    return( 0 );

}


static int coreShareReadQueue( SHARED_MSG_QUEUE_DS *msgQPtr, uint8_t *msgPtr, int size )
{
         int      tTail;
volatile uint8_t *srcPtr;

    /*  We trust that somebody upstream made sure it will fit, but just to avoid
     *  confusion, ensure we fit.  Requests for larger than we have are errors.
     */
    if( size > msgQPtr->size ) {
        return( -1 );
    }

    /*  See if we have anything, return if no data; callers responsibility to poll.
     */
    if( msgQPtr->tail == msgQPtr->head ) {
	return( 0 );
    }

    /*  Form and bump our tail to next slot, but current slot is where we fetch data
     *  from.  
     */
    tTail = (msgQPtr->tail + 1);
    if( tTail == msgQPtr->count ) {
	tTail = 0;
    }

    /*  Copy data.  The Queue Buffer are msgQPtr->size blocks starting
     *  at msgQPtr->mcbBase, so a physical addressis formed based on entry index,
     *  with that index being what 'head' currently points to.
     */
    srcPtr = &msgQPtr->mcbBase[ msgQPtr->tail * msgQPtr->size ];
    memcpy( (void *)msgPtr, (void *)srcPtr, size );

    /* Update tail to reflect we've read.
     */
    msgQPtr->tail = tTail;

}



#ifdef  __X86_TESTING__
int main()
{
    printf( "Groovy\n" );
    coreShareInit();
}
#endif
