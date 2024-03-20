/**
 * Simple memory allocation tracking tool till
 * I can figure out something better.
 */

extern void* MemNew(void* obj, const char* className, int size);
extern void MemDelete(void* obj, const char* name);

#if 1

#define NEW(cls) (cls*)MemNew(new cls, #cls, sizeof(cls))

#define NEW(cls, arg1) (cls*)MemNew(new cls(arg1), #cls, sizeof(cls))

#else

#define NEW(cls) new cls

#define NEW(cls, arg1) new cls(arg1)

#endif
