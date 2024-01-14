/**
 * Simple debug output utilities.
 * Used because Juce DBG() seems to have a lot of overhead, not sure why.
 * OutputDebugString isn't that fast either, will need to port over the
 * original queued output trace eventually.
 */

// this was taken from the original Mobius Trace file, don't need all of these yet
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

// only works on Windows, Mac port eventually
#include <io.h>
#include <windows.h>

void Trace(const char* str)
{
    OutputDebugString(str);
}
