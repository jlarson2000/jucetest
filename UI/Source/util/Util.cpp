/*
 * Copyright (c) 2024 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Yet another collection of utilities.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Util.h"

//////////////////////////////////////////////////////////////////////
//
// String utilities
//
//////////////////////////////////////////////////////////////////////

/**
 * Copy a string to a point.
 */
char* CopyString(const char* src, int len)
{
	char* copy = NULL;
	if (src != NULL && len > 0) {
		int srclen = strlen(src);
		if (len <= srclen) {
			copy = new char[len + 1];
			if (copy != NULL) {
				strncpy(copy, src, len);
				copy[len] = 0;
			}
		}
	}
	return copy;
}

/**
 * Copy one string to a buffer with care.
 * The max argument is assumed to be the maximum number
 * of char elements in the dest array *including* the nul terminator.
 *
 * TODO: Some of the applications of this would like us to favor
 * the end rather than the start.
 */
void CopyString(const char* src, char* dest, int max)
{
	if (dest != nullptr && max > 0) {
		if (src == nullptr) 
		  strcpy(dest, "");
		else {
			int len = (int)strlen(src);
			int avail = max - 1;
			if (avail > len)
			  strcpy(dest, src);
			else {
				strncpy(dest, src, avail);
				dest[avail] = 0;
			}
		}
	}
}

/**
 * CopyString
 *
 * Return a copy of a string allocated with new.
 * Returns nullptr if nullptr is passed.
 * 
 * strdup() doesn't handle nullptr input pointers, and the result is supposed
 * to be freed with free() not the delete operator.  Many compilers don't
 * make a distinction between free() and delete, but that is not guarenteed.
 */
char* CopyString(const char *src)
{
	char *copy = nullptr;
	if (src) {
		copy = new char[strlen(src) + 1];
		if (copy)
		  strcpy(copy, src);
	}
	return copy;
}

void AppendString(const char* src, char* dest, int max)
{
    if (src != NULL) {
        int current = strlen(dest);
        int neu = strlen(src);
        int avail = max - 1;
        if (avail > current + neu)
          strcat(dest, src);
    }
}

int LastIndexOf(const char* str, const char* substr)
{
	int index = -1;

	// not a very smart search
	if (str != NULL && substr != NULL) {
		int len = strlen(str);
		int sublen = strlen(substr);
		int psn = len - sublen;
		if (psn >= 0) {
			while (psn >= 0 && strncmp(&str[psn], substr, sublen))
			  psn--;
			index = psn;
		}
	}
	return index;
}

/**
 * Case insensitive string comparison.
 * Return true if the strings are equal.
 */
bool StringEqualNoCase(const char* s1, const char* s2)
{	
	bool equal = false;
	
	if (s1 == NULL) {
		if (s2 == NULL)
		  equal = true;
	}
	else if (s2 != NULL) {
		int len = strlen(s1);
		int len2 = strlen(s2);
		if (len == len2) {
			equal = true;
			for (int i = 0 ; i < len ; i++) {
				char ch = tolower(s1[i]);
				char ch2 = tolower(s2[i]);
				if (ch != ch2) {
					equal = false;
					break;
				}
			}
		}

	}
	return equal;
}

/**
 * String comparison handling nulls.
 */
bool StringEqual(const char* s1, const char* s2)
{	
	bool equal = false;
	
	if (s1 == NULL) {
		if (s2 == NULL)
		  equal = true;
	}
	else if (s2 != NULL)
	  equal = !strcmp(s1, s2);

	return equal;
}

bool StringEqualNoCase(const char* s1, const char* s2, int max)
{	
	bool equal = false;
	
	if (s1 == NULL) {
		if (s2 == NULL)
		  equal = true;
	}
	else if (s2 != NULL) {
		int len = strlen(s1);
		int len2 = strlen(s2);
        if (len >= max && len2 >= max) {
			equal = true;
			for (int i = 0 ; i < max ; i++) {
				char ch = tolower(s1[i]);
				char ch2 = tolower(s2[i]);
				if (ch != ch2) {
					equal = false;
					break;
				}
			}
		}

	}
	return equal;
}

/**
 * Given a string of numbers, either whitespace or comma delimited, 
 * parse it and build an array of ints.  Return the number of ints
 * parsed.
 */
int ParseNumberString(const char* src, int* numbers, int max)
{
	char buffer[MAX_NUMBER_TOKEN + 1];
	const char* ptr = src;
	int parsed = 0;

	if (src != NULL) {
		while (*ptr != 0 && parsed < max) {
			// skip whitespace
			while (*ptr != 0 && isspace(*ptr) && !(*ptr == ',')) ptr++;

			// isolate the number
			char* psn = buffer;
			for (int i = 0 ; *ptr != 0 && i < MAX_NUMBER_TOKEN ; i++) {
				if (isspace(*ptr) || *ptr == ',') {
					ptr++;
					break;
				}
				else
				  *psn++ = *ptr++;
			}
			*psn = 0;

			if (strlen(buffer) > 0) {
				int ival = atoi(buffer);
				if (numbers != NULL)
				  numbers[parsed] = ival;
				parsed++;
			}
		}
	}

	return parsed;
}

bool StartsWith(const char* str, const char* prefix)
{
	bool startsWith = false;
	if (str != NULL && prefix != NULL)
      startsWith = !strncmp(str, prefix, strlen(prefix));
    return startsWith;
}

bool StartsWithNoCase(const char* str, const char* prefix)
{
	bool startsWith = false;
	if (str != NULL && prefix != NULL)
        startsWith = StringEqualNoCase(str, prefix, strlen(prefix));
	return startsWith;
}

bool EndsWith(const char* str, const char* suffix)
{
	bool endsWith = false;
	if (str != NULL && suffix != NULL) {
		int len1 = strlen(str);
		int len2 = strlen(suffix);
		if (len1 > len2)
		  endsWith = !strcmp(suffix, &str[len1 - len2]);
	}
	return endsWith;
}

bool EndsWithNoCase(const char* str, const char* suffix)
{
	bool endsWith = false;
	if (str != NULL && suffix != NULL) {
		int len1 = strlen(str);
		int len2 = strlen(suffix);
		if (len1 > len2)
		  endsWith = StringEqualNoCase(suffix, &str[len1 - len2]);
	}
	return endsWith;
}

/**
 * Necessary because atoi() doesn't accept NULL arguments.
 */
int ToInt(const char* str)
{
	int value = 0;
	if (str != NULL)
	  value = atoi(str);
	return value;
}

/**
 * Return true if the string looks like a signed integer.
 */
bool IsInteger(const char* str)
{
    bool is = false;
    if (str != NULL) {
        int max = strlen(str);
        if (max > 0) {
            is = true;
            for (int i = 0 ; i < max && is ; i++) {
                char ch = str[i];
                if (!isdigit(ch) && (i > 0 || ch != '-'))
                  is = false;
            }
        }
    }
    return is;
}

void GetLeafName(const char* path, char* buffer, bool extension)
{
	int last = strlen(path) - 1;
	int dot = -1;
	int psn = last;

	while (psn > 0 && path[psn] != '/' && path[psn] != '\\') {
		if (path[psn] == '.')
		  dot = psn;
		psn--;
	}

	if (psn < 0) {
		// looked like a simple file name, no change
		psn = 0;
	}
	else
	  psn++;

	if (!extension && dot > 0)
	  last = dot - 1;

	int len = last - psn + 1;

	strncpy(buffer, &path[psn], len);
	buffer[len] = 0;
}

int IndexOf(const char* str, const char* substr)
{
    return IndexOf(str, substr, 0);
}

int IndexOf(const char* str, const char* substr, int start)
{
	int index = -1;

	// not a very smart search
	if (str != NULL && substr != NULL) {
		int len = strlen(str);
		int sublen = strlen(substr);
        int max = len - sublen;
        if (sublen > 0 && max >= 0) {
            for (int i = 0 ; i <= max ; i++) {
                if (strncmp(&str[i], substr, sublen) == 0) {
                    index = i;
                    break;
                }
            }
        }
	}
	return index;
}

//////////////////////////////////////////////////////////////////////
//
// Exceptions
//
// This was originally used by the XML parser, not sure how much it
// has grown since then.
//
// Note that since this does dynamic allocation it is unsuitable for
// use in the audio interrupt.  Need to replace this with something better
// from the standard library.
//
//////////////////////////////////////////////////////////////////////

AppException::AppException(const char *msg, bool nocopy)
{
	AppException(ERR_GENERIC, msg, nocopy);
}

AppException::AppException(int c, const char *msg, bool nocopy)
{
	mCode = c;
	if (nocopy)
	  mMessage = (char *)msg;
	else if (msg == nullptr) 
	  mMessage = nullptr;
	else {
		// if this throws, then I guess its more important 
		mMessage = new char[strlen(msg) + 1];
		strcpy(mMessage, msg);
	}	
}

/** 
 * copy constructor, important for this exceptions since
 * we want to pass ownership of the string
 */
AppException::AppException(AppException &src) 
{
	mCode = src.mCode;
	mMessage = src.mMessage;
	src.mMessage = nullptr;
}

AppException::~AppException(void)
{
	delete mMessage;
}

void AppException::print(void)
{
	if (mMessage)
	  printf("ERROR %d : %s\n", mCode, mMessage);
	else
	  printf("ERROR %d\n", mCode);
}

//////////////////////////////////////////////////////

static bool RandomSeeded = false;

/**
 * Generate a random number between the two values, inclusive.
 */
int Random(int min, int max)
{
	// !! potential csect issues here
	if (!RandomSeeded) {
		// passing 1 "reinitializes the generator", passing any other number
		// "sets the generator to a random starting point"
		// Unclear how the seed affects the starting point, probably should
		// be based on something, maybe pass in the layer size?
		srand(2);
		RandomSeeded = true;
	}

	int range = max - min + 1;
	int value = (rand() % range) + min;

	return value;
}
