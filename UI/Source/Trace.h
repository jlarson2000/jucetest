/**
 * Simple debug output utilities.
 * Used because Juce DBG() seems to have a lot of overhead, not sure why.
 */
#pragma once

#include <string>
#include <sstream>

void Trace(const char* buf);
void Trace(std::string* str);
void Trace(std::ostringstream* os);

