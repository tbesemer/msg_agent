
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <core_share.h>


static void  traceLogInit();

static int sharedMemFd;
volatile static void *sharedMemPtr;

volatile static TRACE_LOG_DS *traceBufferPtr = (TRACE_LOG_DS *)NULL;

void *coreShareGetMemoryBlock( int offset, int size )
{
    return( (void *)(sharedMemPtr + offset) );
}

static int coreLinuxMemAttach( SHARE_MEM_TYPES type )
{

    sharedMemFd = open("/dev/mem", O_RDWR | O_SYNC);
    if( sharedMemFd < 0 ) {
        fprintf( stdout, "corelinuxMemAttach(): open on /dev/mem failed\n" );
        return( -1 );
    }

    sharedMemPtr = mmap( 0, SHARE_ALLOC_MEM_SIZE, (PROT_READ | PROT_WRITE),
                  MAP_SHARED, sharedMemFd, (SHARE_ALLOC_MEM_ADDR & ~SHARE_ALLOC_MEM_MASK) );
    if( sharedMemPtr == (void *)-1 ) {
        fprintf( stdout, "coreLinuxMemAttach(): mmap() failed\n" );
        close( sharedMemFd );
        return( -1 );
    }

    fprintf( stdout, "coreLinuxMemAttach(): sharedMemPtr= 0x%08X\n", (int)sharedMemPtr );

    return( 0 );
}


int coreShareInit( SHARE_MEM_TYPES type )
{

    if( coreLinuxMemAttach( type ) < 0 ) {
        fprintf( stderr, "coreShareInit(): coreLinuxMemAttach() Failed\n" );
	return( -1 );
    }

    traceLogInit();
    coreShareInitMsgQueues( type );

    return( 0 );
}

static void  traceLogInit()
{

    traceBufferPtr = (TRACE_LOG_DS *)coreShareGetMemoryBlock( SHARE_TRACE_BUFFER_OFFSET, sizeof(TRACE_LOG_DS) );

}

volatile TRACE_LOG_ENTRY_DS *getTraceEntry()
{
volatile TRACE_LOG_ENTRY_DS  *logEntry;

    if( traceBufferPtr->head != traceBufferPtr->tail ) {
        logEntry = &traceBufferPtr->traceLog[ traceBufferPtr->tail++ ];
        if( traceBufferPtr->tail == NUM_TRACE_BUFFERS ) {
            traceBufferPtr->tail = 0;
        }
	return( logEntry );
    } else {
        return( (TRACE_LOG_ENTRY_DS *)NULL );
    } 

}

int coreShareReadBlock( char *blockPtr, int block, int blockSize )
{
void *tPtr;

    tPtr = coreShareGetMemoryBlock( (block * blockSize), blockSize );
    memcpy( (void *)blockPtr, tPtr, blockSize );

}

