/*
 * Copyright (c) 2024 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * !! Critical section support has been commented out pending design of
 * how to integgrate Juce for this
 *
 * Trace utilities.
 * 
 * This is used to send messages to the Windows debug output stream which
 * is necessary since stdout is not accessible when running as a plugin.
 * Stdout is also not visible when running under Visual Studio.
 *
 * Mac doesn't have this, I've been just using stdout/stderr and doing most
 * development on Windows.  With the addition of Juce it would be interesting
 * to explore having a Juce window async window that could be used for this
 * on both platforms.
 *
 * This is called from the audio interrupt and must be fast and not do
 * anything dangerous like allocating dynamic memory.
 *
 * For both of these reasons trace records are normally simply accumulated
 * in a global array which is then sent to the appropriate display method
 * by another thread outside the interrupt.
 *
 * Adding trace records should be synchronized since records can be added
 * by concurrent threads.  In practice there will only be one thread pulling
 * records out of the queue.
 *
 * The records are acumulated as a ring buffer with a head
 * pointer advanced as trace messages are added, and a tail pointer advanced
 * as messages are displayed.  This can be bypassed for testing purposes
 * by setting the TracetoStdout flag.
 *
 */

#include <stdio.h>
#include <stdarg.h>

// why is this not defined?
#define _WIN32 1

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#endif

// Used for CriticalSection, need to work on this
// it was leaking
//#include "Thread.h"

#include "Util.h"
#include "Trace.h"

/****************************************************************************
 *                                                                          *
 *   							 SIMPLE TRACE                               *
 *                                                                          *
 ****************************************************************************/

bool TraceToDebug = true;
bool TraceToStdout = false;

void vtrace(const char *string, va_list args)
{
	char buf[1024];
	vsprintf(buf, string, args);

	if (TraceToStdout) {
		printf("%s", buf);
		fflush(stdout);
	}

	if (TraceToDebug) {
#ifdef _WIN32
		OutputDebugString(buf);
#else
		// OSX sadly doesn't seem to have anything equivalent emit to stderr
		// if we're not already emitting to stdout
		if (!TraceToStdout) {
			fprintf(stderr, "%s", buf);
			fflush(stderr);
		}
#endif
	}

}

void trace(const char *string, ...)
{
    va_list args;
    va_start(args, string);
	vtrace(string, args);
    va_end(args);
}

/****************************************************************************
 *                                                                          *
 *   							 TRACE BUFFER                               *
 *                                                                          *
 ****************************************************************************/

TraceBuffer::TraceBuffer()
{
	mIndent = 0;
}

TraceBuffer::~TraceBuffer()
{
}

void TraceBuffer::incIndent()
{
	mIndent += 2;
}

void TraceBuffer::decIndent()
{
	mIndent -= 2;
}

void TraceBuffer::add(const char *string, ...)
{
    va_list args;
    va_start(args, string);
	addv(string, args);
    va_end(args);
}

void TraceBuffer::addv(const char *string, va_list args)
{
	char buf[1024];
	vsprintf(buf, string, args);

	for (int i = 0 ; i < mIndent ; i++)
	  printf(" ");
	printf("%s", buf);
}

void TraceBuffer::print()
{
	fflush(stdout);
}

/****************************************************************************
 *                                                                          *
 *   							TRACE RECORDS                               *
 *                                                                          *
 ****************************************************************************/
/*
 * Trace mechanism optimized for the gathering of potenitally
 * large amounts of trace data, such as in digital audio processing.
 */

/**
 * Trace records at this level or lower are printed to the console.
 */
int TracePrintLevel = 1;

/**
 * Trace records at this level or lower are sent to the debug output stream.
 */
int TraceDebugLevel = 1;

/**
 * When set, trace messages for both the print and debug streams
 * are queued, and the listener is notified.  The listener is expected
 * to call FlushTrace eventually in another thread.
 */
TraceListener* NewTraceListener = nullptr;

/**
 * Trace records are accumulated in a global array.
 * In theory there could be thread synchronization problems, 
 * but in practice that would be rare as almost all trace messages
 * come from the interrupt thread.  I don't want to mess with
 * csects here, the only potential problem is loss of a message.
 */
TraceRecord TraceRecords[MAX_TRACE_RECORDS];

/**
 * Csect needed for the head pointer since we can be adding
 * trace from several threads.
 * You must only flush trace from one thread.
 */

//jsl - need to figure something out here that doesn't leak
//CriticalSection* TraceCsect = new CriticalSection("Trace");

/**
 * Index into TraceRecords of the first active record.
 * If this is equal to TraceTail, then the message queue is empty.
 */
int TraceHead = 0;

/**
 * The index into TraceRecords of the next available record.
 */
int TraceTail = 0;

/**
 * A default object that may be registered to provide context and time
 * info for all trace records.
 */
TraceContext* DefaultTraceContext = nullptr;

bool TraceInitialized = false;

void TraceBreakpoint()
{
	int x = 0;
}

void ResetTrace()
{
//    TraceCsect->enter();
	TraceHead = 0;
	TraceTail = 0;
//    TraceCsect->leave();
}

/**
 * Fix an argument to it is safe to copy.
 * 
 * Note that we can't tell the difference between NULL and 
 * empty string once we copy, which is important in order to select
 * the right sprintf argument list.  If either of these are 
 * non-null but empty, convert them to a single space so we know
 * that a string is expected at this position.
 */
void SaveArgument(const char* src, char* dest)
{
    dest[0] = 0;
    if (src != nullptr) {
        if (strlen(src) == 0) 
          src = " ";
        else if ((unsigned long)src < 65535)
          src = "INVALID";
        CopyString(src, dest, MAX_ARG);
        
    }
}

/**
 * Add a trace record to the trace array.
 * If we're queueing and we fill the record array, we can either lose
 * old records or new ones.  New ones are more important, but we're not
 * supposed to increment TraceHead if TraceQueued is on.
 * 
 */
void AddTrace(TraceContext* context, int level, 
              const char* msg, 
              const char* string1, 
              const char* string2,
              const char* string3,
              long l1, long l2, long l3, long l4, long l5)
{
	// kludge: trying to track down a problem, make sure the 
	// records are initialized
	if (!TraceInitialized) {
		for (int i = 0 ; i < MAX_TRACE_RECORDS ; i++)
			TraceRecords[i].msg = nullptr;
		TraceInitialized =  true;
	}

	// only queue if it falls within the interesting levels
	if (level <= TracePrintLevel || level <= TraceDebugLevel) {

        // must csect this, the TraceTail can't advance
        // until we've fully initialized the record or else
        // the flush thread can try to render a partially
        // initialized record
//        TraceCsect->enter();

		TraceRecord* r = &TraceRecords[TraceTail];

		int nextTail = TraceTail + 1;
		if (nextTail >= MAX_TRACE_RECORDS)
		  nextTail = 0;

		if (nextTail == TraceHead) {
            // overflow
            // originally we bumped the head but that causes
            // problems if the flush thread is active at the moment,
            // we can overwtite the record being flushed causing
            // a sprintf format/argument mismatch.  This can
            // only happen when the refresh thread is bogged down 
            // or excessive trace is being generated
//            TraceCsect->leave();
            const char* warning = "WARNING: Trace record buffer overflow!!\n";
#ifdef _WIN32
			OutputDebugString(warning);
            fprintf(stdout, warning);
            fflush(stdout);
#else
            fprintf(stderr, warning);
            fflush(stderr);
#endif
		}
        else {
            // use the default context if none explictily passedn
            if (context == nullptr)
              context = DefaultTraceContext;

            if (context != nullptr)
              context->getTraceContext(&(r->context), &(r->time));
            else {
                r->context = 0;
                r->time = 0;
            }

            r->level = level;
            r->msg = msg;
            r->long1 = l1;
            r->long2 = l2;
            r->long3 = l3;
            r->long4 = l4;
            r->long5 = l5;
            r->string[0] = 0;
            r->string2[0] = 0;
            r->string3[0] = 0;

            try {
                SaveArgument(string1, r->string);
                SaveArgument(string2, r->string2);
                SaveArgument(string3, r->string3);
            }
            catch (...) {
                printf("Trace: Unable to copy string arguments!\n");
            }

            // only change the tail after the record is fully initialized
            TraceTail = nextTail;
//            TraceCsect->leave();
        }

		// spot to hang a breakpoint
		if (level <= 1)
		  TraceBreakpoint();
	}
}

/**
 * Render the contents of a trace record to a character buffer.
 * !! need max handling.
 */
void RenderTrace(TraceRecord* r, char* buffer)
{
	if (r->msg == nullptr)
	  sprintf(buffer, "ERROR: Invalid trace message!\n");
	else {
        try {
            sprintf(buffer, "%s%d %ld: ", ((r->level == 1) ? "ERROR: " : ""),
                    r->context, r->time);
            buffer += strlen(buffer);
	

            if (strlen(r->string3) > 0) {
                sprintf(buffer, r->msg, r->string, r->string2, r->string3,
                        r->long1, r->long2, r->long3, r->long4, r->long5);
            }
            else if (strlen(r->string2) > 0) {
                sprintf(buffer, r->msg, r->string, r->string2, 
                        r->long1, r->long2, r->long3, r->long4, r->long5);
            }
            else if (strlen(r->string) > 0)
              sprintf(buffer, r->msg, r->string, 
                      r->long1, r->long2, r->long3, r->long4, r->long5);
            else
              sprintf(buffer, r->msg, r->long1, r->long2, r->long3, r->long4, r->long5);
        }
        catch (...) {
            // don't let malformed trace args ruin your day
            // actually this will rarely work because SEGV or misalligned
            // pointers raise signals rather than throw exceptions
            sprintf(buffer, "ERROR: Exception rendering trace: %s\n", r->msg);
        }

        // this is so easy to miss
        int len = strlen(buffer);
        if (len > 0) {
            char last = buffer[len-1];
            if (last != '\n') {
                buffer[len] = '\n';
                buffer[len+1] = 0;
            }
        }

		// keep this clear so we can try to detect anomolies in the
		// head/tail iteration
		r->msg = nullptr;
	}
}

/****************************************************************************
 *                                                                          *
 *   						BUFFERED TRACE OUTPUT                           *
 *                                                                          *
 ****************************************************************************/

void WriteTrace(FILE* fp)
{
	char buffer[1024 * 8];

    fprintf(fp, "=========================================================\n");
	while (TraceHead != TraceTail) {
        TraceRecord* r = &TraceRecords[TraceHead];
		RenderTrace(r, buffer);
		fprintf(fp, "%s", buffer);
		TraceHead++;
		if (TraceHead >= MAX_TRACE_RECORDS)
		  TraceHead = 0;
    }
}

void WriteTrace(const char* file)
{
	if (TraceHead != TraceTail > 0) {
		FILE* fp = fopen(file, "w");
		if (fp != nullptr) {
			WriteTrace(fp);
			fclose(fp);
		}
		else
		  printf("Unable to open trace output file %s\n", file);
	}
}

void AppendTrace(const char* file)
{
	if (TraceHead != TraceTail) {
		FILE* fp = fopen(file, "a");
		if (fp != nullptr) {
			WriteTrace(fp);
			fclose(fp);
		}
		else
		  printf("Unable to open trace output file %s\n", file);
	}
}

void PrintTrace()
{
    WriteTrace(stdout);
}

void FlushTrace()
{
	char buffer[1024 * 8];
	
	// guard against mods during the flush, really that safe?
	int head = TraceHead;
	int tail = TraceTail;

	while (head != tail) {
        TraceRecord* r = &TraceRecords[head];
		RenderTrace(r, buffer);

		if (r->level <= TracePrintLevel) {
			printf("%s", buffer);
			fflush(stdout);
		}

		if (r->level <= TraceDebugLevel) {

#ifdef _WIN32
			OutputDebugString(buffer);
#else
			// OSX sadly doesn't seem to have anything equivalent emit to stderr
			// if we're not already emitting to stdout
			if (!(r->level <= TracePrintLevel)) {
				fprintf(stderr, "%s", buffer);
				fflush(stderr);
			}
#endif
		}

		head++;
		if (head >= MAX_TRACE_RECORDS)
		  head = 0;
    }

	TraceHead = head;
}

/**
 * Flush the messages or notify the listener
 */
void FlushOrNotify()
{
	if (NewTraceListener != nullptr)
	  NewTraceListener->traceEvent();
	else
	  FlushTrace();
}

/****************************************************************************
 *                                                                          *
 *   							TRACE METHODS                               *
 *                                                                          *
 ****************************************************************************/

/**
 * Called for every string argument to make sure that it has a value,
 * otherwise when the trace record is rendered we pick the wrong
 * set of args for the format string.
 */
const char* CheckString(const char* arg)
{
    if (arg == nullptr || strlen(arg) == 0) {
        // originally had "???" and then "null"
        // but this happens in a few places where we just don't
        // want to display anything, so leave empty
        arg = "";
    }
    return arg;
}

void Trace(int level, const char* msg)
{
	Trace(nullptr, level, msg);
}

void Trace(TraceContext* context, int level, const char* msg)
{
    AddTrace(context, level, msg, nullptr, nullptr, nullptr, 0, 0, 0, 0, 0);
	FlushOrNotify();
}

void Trace(int level, const char* msg, const char* arg)
{
	Trace(nullptr, level, msg, arg);
}

void Trace(TraceContext* context, int level, const char* msg, 
				  const char* arg)
{
    arg = CheckString(arg);
	AddTrace(context, level, msg, arg, nullptr, nullptr, 0, 0, 0, 0, 0);
	FlushOrNotify();
}

void Trace(int level, const char* msg, const char* arg,
				  const char* arg2)
{
	Trace(nullptr, level, msg, arg, arg2);
}

void Trace(TraceContext* context, int level, const char* msg, 
				  const char* arg, const char* arg2)
{
    arg = CheckString(arg);
    arg2 = CheckString(arg2);
    AddTrace(context, level, msg, arg, arg2, nullptr, 0, 0, 0, 0, 0);
	FlushOrNotify();
}

void Trace(int level, const char* msg, const char* arg,
				  const char* arg2, const char* arg3)
{
	Trace(nullptr, level, msg, arg, arg2, arg3);
}

void Trace(TraceContext* context, int level, const char* msg, 
				  const char* arg, const char* arg2, const char* arg3)
{
    arg = CheckString(arg);
    arg2 = CheckString(arg2);
    arg3 = CheckString(arg3);
    AddTrace(context, level, msg, arg, arg2, arg3, 0, 0, 0, 0, 0);
	FlushOrNotify();
}

void Trace(int level, const char* msg, const char* arg, long l1)
{
	Trace(nullptr, level, msg, arg, l1);
}

void Trace(TraceContext* context, int level, const char* msg, 
				  const char* arg, long l1)
{
    arg = CheckString(arg);
    AddTrace(context, level, msg, arg, nullptr, nullptr, l1, 0, 0, 0, 0);
	FlushOrNotify();
}

void Trace(int level, const char* msg, 
				  const char* arg, const char* arg2, long l1)
{
	Trace(nullptr, level, msg, arg, arg2, l1);
}

void Trace(int level, const char* msg, 
				  const char* arg, const char* arg2, long l1, long l2)
{
	Trace(nullptr, level, msg, arg, arg2, l1, l2);
}

void Trace(int level, const char* msg, 
				  const char* arg, const char* arg2, long l1, long l2, long l3)
{
	Trace(nullptr, level, msg, arg, arg2, l1, l2, l3);
}

void Trace(TraceContext* context, int level, const char* msg, 
				  const char* arg, const char* arg2, long l1)
{
    arg = CheckString(arg);
    arg2 = CheckString(arg2);
    AddTrace(context, level, msg, arg, arg2, nullptr, l1, 0, 0, 0, 0);
	FlushOrNotify();
}

void Trace(TraceContext* context, int level, const char* msg, 
				  const char* arg, const char* arg2, long l1, long l2)
{
    arg = CheckString(arg);
    arg2 = CheckString(arg2);
    AddTrace(context, level, msg, arg, arg2, nullptr, l1, l2, 0, 0, 0);
	FlushOrNotify();
}

void Trace(TraceContext* context, int level, const char* msg, 
				  const char* arg, const char* arg2, long l1, long l2, long l3)
{
    arg = CheckString(arg);
    arg2 = CheckString(arg2);
    AddTrace(context, level, msg, arg, arg2, nullptr, l1, l2, l3, 0, 0);
	FlushOrNotify();
}

void Trace(int level, const char* msg, 
				  const char* arg, long l1, long l2)
{
	Trace(nullptr, level, msg, arg, l1, l2);
}

void Trace(TraceContext* context, int level, const char* msg, 
				  const char* arg, long l1, long l2)
{
    arg = CheckString(arg);
    AddTrace(context, level, msg, arg, nullptr, nullptr, l1, l2, 0, 0, 0);
	FlushOrNotify();
}

void Trace(int level, const char* msg, long l1)
{
	Trace(nullptr, level, msg, l1);
}

void Trace(TraceContext* context, int level, const char* msg, long l1)
{
    AddTrace(context, level, msg, nullptr, nullptr, nullptr, l1, 0, 0, 0, 0);
	FlushOrNotify();
}

void Trace(int level, const char* msg, long l1, long l2)
{
	Trace(nullptr, level, msg, l1, l2);
}

void Trace(TraceContext* context, int level, const char* msg, 
				  long l1, long l2)
{
    AddTrace(context, level, msg, nullptr, nullptr, nullptr, l1, l2, 0, 0, 0);
	FlushOrNotify();
}

void Trace(int level, const char* msg, 
				  long l1, long l2, long l3)
{
	Trace(nullptr, level, msg, l1, l2, l3);
}

void Trace(TraceContext* context, int level, const char* msg, 
				  long l1, long l2, long l3)
{
    AddTrace(context, level, msg, nullptr, nullptr, nullptr, l1, l2, l3, 0, 0);
	FlushOrNotify();
}

void Trace(int level, const char* msg, const char* arg,
				  long l1, long l2, long l3)
{
	Trace(nullptr, level, msg, arg, l1, l2, l3);
}

void Trace(TraceContext* context, int level, const char* msg, 
				  const char* arg,
				  long l1, long l2, long l3)
{
    arg = CheckString(arg);
    AddTrace(context, level, msg, arg, nullptr, nullptr, l1, l2, l3, 0, 0);
	FlushOrNotify();
}

void Trace(int level, const char* msg, 
				  long l1, long l2, long l3, long l4)
{
	Trace(nullptr, level, msg, l1, l2, l3, l4);
}

void Trace(TraceContext* context, int level, const char* msg, 
				  long l1, long l2, long l3, long l4)
{
    AddTrace(context, level, msg, nullptr, nullptr, nullptr, l1, l2, l3, l4, 0);
	FlushOrNotify();
}

void Trace(TraceContext* context, int level, const char* msg, 
				  const char* arg,
				  long l1, long l2, long l3, long l4)
{
    arg = CheckString(arg);
    AddTrace(context, level, msg, arg, nullptr, nullptr, l1, l2, l3, l4, 0);
	FlushOrNotify();
}

void Trace(int level, const char* msg, 
                  const char* arg,
				  long l1, long l2, long l3, long l4)
{
	Trace(nullptr, level, msg, arg, l1, l2, l3, l4);
}

void Trace(int level, const char* msg, 
				  long l1, long l2, long l3, long l4, long l5)
{
	Trace(nullptr, level, msg, l1, l2, l3, l4, l5);
}

void Trace(int level, const char* msg, const char* arg,
				  long l1, long l2, long l3, long l4, long l5)
{
	Trace(nullptr, level, msg, arg, l1, l2, l3, l4, l5);
}

void Trace(TraceContext* context, int level, const char* msg, 
				  long l1, long l2, long l3, long l4, long l5)
{
    Trace(context, level, msg, nullptr, l1, l2, l3, l4, l5);
}

void Trace(TraceContext* context, int level, const char* msg, 
                  const char* arg,
                  long l1, long l2, long l3, long l4, long l5)
{
    arg = CheckString(arg);
    AddTrace(context, level, msg, arg, nullptr, nullptr, l1, l2, l3, l4, l5);
	FlushOrNotify();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
