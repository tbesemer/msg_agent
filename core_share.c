
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <core_share.h>


volatile static SHARED_MSG_QUEUE_MGR_DS *queueCtlManagerPtr = (SHARED_MSG_QUEUE_MGR_DS *)NULL;

void coreShareInitMsgQueues()
{

    queueCtlManagerPtr = (SHARED_MSG_QUEUE_MGR_DS *)coreShareGetMemoryBlock( SHARE_MSG_SYSTEM_OFFSET, sizeof(SHARED_MSG_QUEUE_MGR_DS) );

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

}

int coreShareWriteQueue( volatile SHARED_MSG_QUEUE_DS *msgQPtr, uint8_t *msgPtr, int size )
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


volatile SHARED_MSG_QUEUE_DS *coreShareGetTxQueue()
{
    return( &queueCtlManagerPtr->txQueueCtl );
}

volatile SHARED_MSG_QUEUE_DS *coreShareGetRxQueue()
{
    return( &queueCtlManagerPtr->rxQueueCtl );
}

