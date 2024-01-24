/**
 * Simple debug output utilities.
 * Used because Juce DBG() seems to have a lot of overhead, not sure why.
 * OutputDebugString isn't that fast either, will need to port over the
 * original queued output trace eventually.
 */

// necessary for OutputDebugString
// debugapi.h alone didn't work, get some kind of missing architecture error
// windows.h seems to cure that
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <windows.h>

#include <string>
#include <sstream>

// this was taken from the original Mobius Trace file, don't need all of these yet
/*
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
*/

void Trace(const char* str)
{
    OutputDebugString(str);
}

void Trace(std::string* s)
{
    Trace(s->c_str());
}

void Trace(std::ostringstream* os)
{
    Trace(os->str().c_str());
}
