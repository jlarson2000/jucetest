/*
 * Various old utilities
 * Replace with std:: eventually
 */

#include <stdio.h>
#include <string.h>

#include "Util.h"

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
	if (dest != NULL && max > 0) {
		if (src == NULL) 
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
 * Arguments:
 *	src: string to copy
 *
 * Returns: copied string (allocated with new)
 *
 * Description: 
 * 
 * The one function that should be in everyone's damn C++ run time library.
 * Copies a null terminated string.  Uses the "new" operator to allocate
 * storage.  Returns NULL if a NULL pointer is passed.
 * 
 * strdup() doesn't handle NULL input pointers, and the result is supposed
 * to be freed with free() not the delete operator.  Many compilers don't
 * make a distinction between free() and delete, but that is not guarenteed.
 */

char* CopyString(const char *src)
{
	char *copy = NULL;
	if (src) {
		copy = new char[strlen(src) + 1];
		if (copy)
		  strcpy(copy, src);
	}
	return copy;
}

/****************************************************************************
 *                                                                          *
 *   							  EXCEPTIONS                                *
 *                                                                          *
 ****************************************************************************/

// needed by the Xml/Xom parser

AppException::AppException(const char *msg, bool nocopy)
{
	AppException(ERR_GENERIC, msg, nocopy);
}

AppException::AppException(int c, const char *msg, bool nocopy)
{
	mCode = c;
	if (nocopy)
	  mMessage = (char *)msg;
	else if (msg == NULL) 
	  mMessage = NULL;
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
	src.mMessage = NULL;
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

