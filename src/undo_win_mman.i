#include <windows.h>

#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

#define MAP_PRIVATE 2
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20

#define MAP_FAILED ((void *)-1)

/* http://www.genesys-e.org/jwalter/mix4win.htm */

/* getpagesize for windows */
long getpagesize (void) {
    static long g_pagesize = 0;
    if (! g_pagesize) {
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        g_pagesize = system_info.dwPageSize;
    }
    return g_pagesize;
}
long getregionsize (void) {
    static long g_regionsize = 0;
    if (! g_regionsize) {
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        g_regionsize = system_info.dwAllocationGranularity;
    }
    return g_regionsize;
}

/* mmap for windows */
void *mmap (void *ptr, long size, long prot, long type, long handle, long arg) {
    static long g_pagesize;
    static long g_regionsize;

    /* First time initialization */
    if (! g_pagesize)
        g_pagesize = getpagesize ();
    if (! g_regionsize)
        g_regionsize = getregionsize ();
    /* Allocate this */
    ptr = VirtualAlloc (ptr, size,
                        MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);
    if (! ptr) {
        ptr = MAP_FAILED;
        goto mmap_exit;
    }
mmap_exit:
    return ptr;
}

/* munmap for windows */
long munmap (void *ptr, long size) {
    static long g_pagesize;
    static long g_regionsize;
    int rc = -1;

    /* First time initialization */
    if (! g_pagesize)
        g_pagesize = getpagesize ();
    if (! g_regionsize)
        g_regionsize = getregionsize ();
    /* Free this */
    if (! VirtualFree (ptr, 0,
                       MEM_RELEASE))
        goto munmap_exit;
    rc = 0;
munmap_exit:
    return rc;
}
