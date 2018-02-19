#ifndef __CORE_SHARE__
#define __CORE_SHARE__

#ifdef  __X86_TESTING__
#define uintptr_t long long
#else
#define uintptr_t uint32_t
#endif


#define SHARED_MSG_RX_QUEUE_COUNT	768
#define SHARED_MSG_TX_QUEUE_COUNT	64
#define SHARED_MSG_QUEUE_SIZE		512

typedef struct {

    volatile int     head;
    volatile int     tail;
    volatile int     count;
    volatile int     size;
    volatile uint8_t *mcbBase;    /*  Message Control Buffers Base Address    */

} SHARED_MSG_QUEUE_DS;

typedef struct {

    volatile SHARED_MSG_QUEUE_DS rxQueueCtl;    /* RX Queue, Core 1 -> Core 0 */
    volatile SHARED_MSG_QUEUE_DS txQueueCtl;    /* TX Queue, Core 0 -> Core 1 */

    volatile uint8_t  rxQueueAlloc[ (SHARED_MSG_QUEUE_SIZE * SHARED_MSG_RX_QUEUE_COUNT) ];
    volatile uint8_t  txQueueAlloc[ (SHARED_MSG_QUEUE_SIZE * SHARED_MSG_TX_QUEUE_COUNT) ];

} SHARED_MSG_QUEUE_MGR_DS;



extern int coreShareInit( int type );
extern void *coreShareGetMemoryBlock( int offset, int size );
extern volatile SHARED_MSG_QUEUE_DS *coreShareGetTxQueue();
extern volatile SHARED_MSG_QUEUE_DS *coreShareGetRxQueue();
extern int coreShareWriteQueue( volatile SHARED_MSG_QUEUE_DS *msgQPtr, uint8_t *msgPtr, int size );
extern int coreShareReadQueue( volatile SHARED_MSG_QUEUE_DS *msgQPtr, uint8_t *msgPtr, int size );



/*  On FreeRTOS, SIZE/ADDR this must mache what is in <lscript.ld>
 *  MASK is used on Linux for mmap() operations.
 */
#define SHARE_ALLOC_MEM_SIZE    0x100000
#define SHARE_ALLOC_MEM_ADDR    0x700000
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
