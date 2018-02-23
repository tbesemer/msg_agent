#ifndef __CORE_SHARE__
#define __CORE_SHARE__

#ifdef  __X86_TESTING__
#define uintptr_t long long
#else
#define uintptr_t uint32_t
#endif

/*  On FreeRTOS, SIZE/ADDR this must mache what is in <lscript.ld>
 *  MASK is used on Linux for mmap() operations.
 */
#define SHARE_ALLOC_MEM_SIZE    0x00100000
#define SHARE_ALLOC_MEM_ADDR    0x00700000
#define SHARE_ALLOC_MEM_MASK	(SHARE_ALLOC_MEM_SIZE - 1)

/*  Offsets in Shared Memory for key structures.
 */
#define SHARE_TRACE_BUFFER_OFFSET     0x00000000
#define SHARE_MSG_SYSTEM_OFFSET       0x00020000

/*  Trace Buffer Configuration.
 */
#define NUM_TRACE_BUFFERS       512

/*  All defines must result in 32 byte alignment for proper Cache
 *  operations.
 */
#define SHARED_MSG_RX_QUEUE_COUNT	768
#define SHARED_MSG_TX_QUEUE_COUNT	64
#define SHARED_MSG_QUEUE_SIZE		512

typedef struct {

    /*  Head and Tail must align on a Cache Line boundary for
     *  proper flush/invalidate operations.
     */
    volatile int     head __attribute__ ((aligned (32)));
    volatile int     tail __attribute__ ((aligned (32)));

    volatile int     count;
    volatile int     size;

    /*  Message Control Buffers Base Address.
     *
     *  Used by FreeRTOS side, contains physical addresses
     *  for Buffers.
      */
    volatile uint8_t *mcbBase __attribute__ ((aligned (32)));

    /*  Virtual Message Control Buffers Base Address.
     *
     *  Used by Linux side, contains Virtual addresses
     *  relative to the base of the control structure.
     */
    volatile uint8_t *vmcbBase __attribute__ ((aligned (32)));

} SHARED_MSG_QUEUE_DS;

typedef struct {

    /* RX Queue, Core 1 -> Core 0
     */
    volatile SHARED_MSG_QUEUE_DS rxQueueCtl;

    /* TX Queue, Core 0 -> Core 1
     */
    volatile SHARED_MSG_QUEUE_DS txQueueCtl;

    /*  Allocation for transfer buffers. Must be specified so they align on 
     *  a 32 byte boundary for Cache operations.
     */
    volatile uint8_t  rxQueueAlloc[ (SHARED_MSG_QUEUE_SIZE * SHARED_MSG_RX_QUEUE_COUNT) ];
    volatile uint8_t  txQueueAlloc[ (SHARED_MSG_QUEUE_SIZE * SHARED_MSG_TX_QUEUE_COUNT) ];

} SHARED_MSG_QUEUE_MGR_DS;



extern void *coreShareGetMemoryBlock( int offset, int size );
extern volatile SHARED_MSG_QUEUE_DS *coreShareGetTxQueue();
extern volatile SHARED_MSG_QUEUE_DS *coreShareGetRxQueue();
extern int coreShareWriteQueue( volatile SHARED_MSG_QUEUE_DS *msgQPtr, uint8_t *msgPtr, int size );
extern int coreShareReadQueue( volatile SHARED_MSG_QUEUE_DS *msgQPtr, uint8_t *msgPtr, int size );

/*  Depending on system type, we do different approaches to attaching memory,
 *  such  as TLB mapping or mmmap().  During the call to coreShareInitSystem(),
 *  we specify memory type.
 */
typedef enum {

    SHARE_MEM_TYPE_FREERTOS = 1,
    SHARE_MEM_TYPE_ARM_LINUX = 2,
    SHARE_MEM_TYPE_X86_LINUX = 3,

} SHARE_MEM_TYPES;

extern SHARE_MEM_TYPES coreShareMemType;
extern int coreShareInit( SHARE_MEM_TYPES type );
extern void coreShareInitSystem( SHARE_MEM_TYPES type );
extern void coreShareInitMsgQueues( SHARE_MEM_TYPES type );



typedef struct {

    volatile uint32_t   upperTimeStamp;
    volatile uint32_t   lowerTimeStamp;
    volatile uint32_t   programCounter;
    volatile uint32_t   userParam;

} TRACE_LOG_ENTRY_DS;

typedef struct {

    volatile int                 head __attribute__ ((aligned (32)));
    volatile int                 tail __attribute__ ((aligned (32)));
    volatile TRACE_LOG_ENTRY_DS  *traceLogEntryPtr[ NUM_TRACE_BUFFERS ];
    volatile TRACE_LOG_ENTRY_DS  traceLog[ NUM_TRACE_BUFFERS ];

} TRACE_LOG_DS;

extern void traceLog( uint32_t programCounter, uint32_t userParam );
extern void traceLogExit( uint32_t programCounter, uint32_t userParam );
extern volatile TRACE_LOG_ENTRY_DS *getTraceEntry();


#endif
