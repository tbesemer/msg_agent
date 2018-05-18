#ifndef __CORE_SHARE__
#define __CORE_SHARE__

#ifdef  __X86_TESTING__
#define uintptr_t long long
#else
#define uintptr_t uint32_t
#endif


#define SHARED_MSG_RX_QUEUE_COUNT	512
#define SHARED_MSG_TX_QUEUE_COUNT	64
#define SHARED_MSG_QUEUE_SIZE		768

#define SHARED_MSG_RX_QUEUE_COUNT_10PPS	10
#define SHARED_MSG_TX_QUEUE_COUNT_10PPS	10
#define SHARED_MSG_QUEUE_SIZE_10PPS	64

typedef struct {

    volatile int     head;
    volatile int     tail;
    volatile int     count;
    volatile int     size;
    volatile uint8_t *mcbBase;    /*  Message Control Buffers Base Address    */
    volatile uint8_t *vmcbBase;   /*  Virtual Message Control Buffers Base Address    */

} SHARED_MSG_QUEUE_DS;

typedef struct {

    /*  Primary Messaging Channel.
     */
    volatile SHARED_MSG_QUEUE_DS rxQueueCtl;    /* RX Queue, Core 1 -> Core 0 */
    volatile SHARED_MSG_QUEUE_DS txQueueCtl;    /* TX Queue, Core 0 -> Core 1 */

    volatile uint8_t  rxQueueAlloc[ (SHARED_MSG_QUEUE_SIZE * SHARED_MSG_RX_QUEUE_COUNT) ];
    volatile uint8_t  txQueueAlloc[ (SHARED_MSG_QUEUE_SIZE * SHARED_MSG_TX_QUEUE_COUNT) ];

    /*  10Hz/PPS Messaging Channel.
     */
    volatile SHARED_MSG_QUEUE_DS rxQueueCtl_10pps;    /* RX Queue, Core 1 -> Core 0 */
    volatile SHARED_MSG_QUEUE_DS txQueueCtl_10pps;    /* TX Queue, Core 0 -> Core 1 */

    volatile uint8_t  rxQueueAlloc_10pps[ (SHARED_MSG_QUEUE_SIZE_10PPS * SHARED_MSG_RX_QUEUE_COUNT_10PPS) ];
    volatile uint8_t  txQueueAlloc_10pps[ (SHARED_MSG_QUEUE_SIZE_10PPS * SHARED_MSG_TX_QUEUE_COUNT_10PPS) ];

} SHARED_MSG_QUEUE_MGR_DS;



extern void *coreShareGetMemoryBlock( int offset, int size );
extern volatile SHARED_MSG_QUEUE_DS *coreShareGetTxQueue();
extern volatile SHARED_MSG_QUEUE_DS *coreShareGetRxQueue();
extern volatile SHARED_MSG_QUEUE_DS *coreShareGetTxQueue_10pps();
extern volatile SHARED_MSG_QUEUE_DS *coreShareGetRxQueue_10pps();
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


/*  On FreeRTOS, SIZE/ADDR this must mache what is in <lscript.ld>
 *  MASK is used on Linux for mmap() operations.
 */
#define SHARE_ALLOC_MEM_SIZE    	0x200000      /* Linux treats at all as 2Mb				*/
#define	SHARE_ALLOC_FREERTOS_MEM_SIZE	0x100000      /* FreeRTOS is 1Mb due to how linker manages segments.	*/
#define SHARE_ALLOC_MEM_ADDR    	0x600000
#define SHARE_ALLOC_MEM_MASK		(SHARE_ALLOC_MEM_SIZE - 1)

/*  Offsets in Shared Memory for key structures.
 */
#ifdef	__FREERTOS__
#define SHARE_TRACE_BUFFER_OFFSET	0x00000000
#define SHARE_MSG_SYSTEM_OFFSET		0x00020000
#else
#define SHARE_CAPTURE_BUFFER_OFFSET	0x00000000
#define SHARE_TRACE_BUFFER_OFFSET	0x00100000
#define SHARE_MSG_SYSTEM_OFFSET		0x00120000
#endif


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

extern void traceLog( uint32_t programCounter, uint32_t userParam );
extern void traceLogExit( uint32_t programCounter, uint32_t userParam );
extern volatile TRACE_LOG_ENTRY_DS *getTraceEntry();


/*  Read raw data out of share area.
 */
extern int coreShareReadBlock( char *blockPtr, int block, int blockSize );


#endif

