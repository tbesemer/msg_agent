
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <core_share.h>

#undef	DEBUG_MSGS

SHARE_MEM_TYPES coreShareMemType = 0;

volatile static SHARED_MSG_QUEUE_MGR_DS *queueCtlManagerPtr = (SHARED_MSG_QUEUE_MGR_DS *)NULL;

static void dumpQueueCtl( volatile SHARED_MSG_QUEUE_DS *msgQPtr )
{
    printf( "msgQPtr->head = %d\n", msgQPtr->head );
    printf( "msgQPtr->tail = %d\n", msgQPtr->tail );
    printf( "msgQPtr->count = %d\n", msgQPtr->count );
    printf( "msgQPtr->size = %d\n", msgQPtr->size );
    printf( "msgQPtr->mcbBase = 0x%08X\n", (int)msgQPtr->mcbBase );
    printf( "msgQPtr->vmcbBase = 0x%08X\n", (int)msgQPtr->vmcbBase );
}

void coreShareInitMsgQueues( SHARE_MEM_TYPES type )
{
    coreShareMemType = type;

    printf( "coreShareInitMsgQueues(): Called, type = %d\n", type );

    queueCtlManagerPtr = (SHARED_MSG_QUEUE_MGR_DS *)coreShareGetMemoryBlock( SHARE_MSG_SYSTEM_OFFSET, sizeof(SHARED_MSG_QUEUE_MGR_DS) );

    /*  FreeRTOS is master, it is responsble for setting up infrastructure relative
     *  to BareMetal side.
     */
    if( type == SHARE_MEM_TYPE_FREERTOS ) {
        queueCtlManagerPtr->rxQueueCtl.head = 0;
        queueCtlManagerPtr->rxQueueCtl.tail = 0;
        queueCtlManagerPtr->rxQueueCtl.count = SHARED_MSG_RX_QUEUE_COUNT;
        queueCtlManagerPtr->rxQueueCtl.size = SHARED_MSG_QUEUE_SIZE;
        queueCtlManagerPtr->rxQueueCtl.mcbBase = queueCtlManagerPtr->rxQueueAlloc;

        queueCtlManagerPtr->txQueueCtl.head = 0;
        queueCtlManagerPtr->txQueueCtl.tail = 0;
        queueCtlManagerPtr->txQueueCtl.count = SHARED_MSG_TX_QUEUE_COUNT;
        queueCtlManagerPtr->txQueueCtl.size = SHARED_MSG_QUEUE_SIZE;
        queueCtlManagerPtr->txQueueCtl.mcbBase = queueCtlManagerPtr->txQueueAlloc;

#ifdef	__FREERTOS__
        Xil_DCacheFlushRange( (uint32_t)queueCtlManagerPtr, sizeof(SHARED_MSG_QUEUE_MGR_DS) );
#endif

    } else {

	/* Virtual Message Control Block address, from Linux perspective.
         */
        queueCtlManagerPtr->rxQueueCtl.vmcbBase = queueCtlManagerPtr->rxQueueAlloc;
        queueCtlManagerPtr->txQueueCtl.vmcbBase = queueCtlManagerPtr->txQueueAlloc;
    }
}

int coreShareWriteQueue( volatile SHARED_MSG_QUEUE_DS *msgQPtr, uint8_t *msgPtr, int size )
{
         int     tHead;
volatile uint8_t *dstPtr;

#ifdef	DEBUG_MSGS
    printf( "coreShareWriteQueue(): called, size = %d\n", size );
#endif

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

#ifdef	DEBUG_MSGS
    printf( "coreShareWriteQueue(): tHead = %d, msgQPtr->tail = %d\n", tHead, msgQPtr->tail );
#endif

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
    if( coreShareMemType == SHARE_MEM_TYPE_FREERTOS ) {
        dstPtr = &msgQPtr->mcbBase[ msgQPtr->head * msgQPtr->size ];
    } else {
        dstPtr = &msgQPtr->vmcbBase[ msgQPtr->head * msgQPtr->size ];
    }

    memcpy( (void *)dstPtr, (void *)msgPtr, size );
#ifdef	__FREERTOS__
    Xil_DCacheFlushRange( (uint32_t)dstPtr, size );
#endif

    /*  Push the new index out, letting readers know new data is available.
     */
    msgQPtr->head = tHead;
#ifdef	__FREERTOS__
    Xil_DCacheFlushRange( (uint32_t)&msgQPtr->head, sizeof(msgQPtr->head) );
#endif

#ifdef	DEBUG_MSGS
    dumpQueueCtl( msgQPtr );
#endif


    return( 0 );

}


int coreShareReadQueue( volatile SHARED_MSG_QUEUE_DS *msgQPtr, uint8_t *msgPtr, int size )
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
#ifdef	__FREERTOS__
    Xil_DCacheInvalidateLine( (uint32_t)&msgQPtr->tail, sizeof(msgQPtr->tail) );
#endif
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
    if( coreShareMemType == SHARE_MEM_TYPE_FREERTOS ) {
        srcPtr = &msgQPtr->mcbBase[ msgQPtr->tail * msgQPtr->size ];
    } else {
        srcPtr = &msgQPtr->vmcbBase[ msgQPtr->tail * msgQPtr->size ];
    }

#ifdef	__FREERTOS__
    Xil_DCacheInvalidateLine( (uint32_t)srcPtr, size );
#endif
    memcpy( (void *)msgPtr, (void *)srcPtr, size );

    /* Update tail to reflect we've read.
     */
    msgQPtr->tail = tTail;
#ifdef	__FREERTOS__
    Xil_DCacheFlushRange( (uint32_t)&msgQPtr->tail, sizeof(msgQPtr->tail) );
#endif

    return( 1 );
}


volatile SHARED_MSG_QUEUE_DS *coreShareGetTxQueue()
{
    return( &queueCtlManagerPtr->txQueueCtl );
}

volatile SHARED_MSG_QUEUE_DS *coreShareGetRxQueue()
{
    return( &queueCtlManagerPtr->rxQueueCtl );
}


void coreShareInitSystem( SHARE_MEM_TYPES type )
{
    printf( "coreShareInitSystem(): Called\n" );
    coreShareInit( type );

}
