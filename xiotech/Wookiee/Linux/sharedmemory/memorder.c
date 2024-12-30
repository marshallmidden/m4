asm(".data");
asm(".global SHMEM_START; SHMEM_START: .long 0");
asm(".global SHMEM_END; SHMEM_END: .long 0");
unsigned int endOfMySharedMem;
unsigned char *pStartOfHeap;
unsigned long startOfBESharedMem, endOfBESharedMem;
unsigned long startOfMySharedMem;
unsigned long PSTARTOFHEAP;
