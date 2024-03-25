
#include "../../util/Trace.h"

#include "Mem.h"

// global variable to control whether we trace or not,
// set when we're processing blocks in the audio thread
bool MemTraceEnabled = false;

void MemTrack(void* obj, const char* className, int size)
{
    if (MemTraceEnabled) {
        // ugh, Trace doesn't support 64 bit pointers
        char buf[1024];
        sprintf(buf, "Memory: Allocated %s size %d %p\n", className, size, obj);
        Trace(2, buf);
    }
}

void* MemNew(void* obj, const char* className, int size)
{
    if (MemTraceEnabled) {
        // ugh, Trace doesn't support 64 bit pointers
        char buf[1024];
        sprintf(buf, "Memory: Allocated %s size %d %p\n", className, size, obj);
        Trace(2, buf);
    }
    return obj;
}

void MemDelete(void* obj, const char* varName)
{
    if (MemTraceEnabled) {
        char buf[1024];
        sprintf(buf, "Memmory: Deleting %s %p\n", varName, obj);
        Trace(2, buf);
    }
}
