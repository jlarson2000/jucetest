
#include "../../util/Trace.h"

#include "Mem.h"

void MemTrack(void* obj, const char* className, int size)
{
    // ugh, Trace doesn't support 64 bit pointers
    char buf[1024];
    sprintf(buf, "Allocated %s size %d %p\n", className, size, obj);
    Trace(1, buf);
}

void* MemNew(void* obj, const char* className, int size)
{
    // ugh, Trace doesn't support 64 bit pointers
    char buf[1024];
    sprintf(buf, "Allocated %s size %d %p\n", className, size, obj);
    Trace(1, buf);
    return obj;
}

void MemDelete(void* obj, const char* varName)
{
    char buf[1024];
    sprintf(buf, "Deleting %s %p\n", varName, obj);
    Trace(1, buf);
}

