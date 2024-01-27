/*
 * Yet another collection of utilities.
 * 
 */

#pragma once

void CopyString(const char* src, char* dest, int max);
char* CopyString(const char *src);

/****************************************************************************
 *                                                                          *
 *   							  EXCEPTIONS                                *
 *                                                                          *
 ****************************************************************************/

/**
 * ERR_BASE_*
 *
 * Description: 
 * 
 * Base numbers for ranges of error codes used by MUSE modules.
 * 
 */

#define ERR_BASE			20000

#define ERR_BASE_GENERAL	ERR_BASE 
#define ERR_BASE_XMLP		ERR_BASE + 100

#define ERR_MEMORY 			ERR_BASE_GENERAL + 1
#define ERR_GENERIC			ERR_BASE_GENERAL + 2

/**
 * A convenient exception class containing a message and/or error code.
 */
class AppException {

  public:
	
	AppException(AppException &src);

	AppException(const char *msg, bool nocopy = false);

	AppException(int c, const char *msg = 0, bool nocopy = false);

	~AppException(void);

	int getCode(void) {
		return mCode;
	}

	const char *getMessage(void) {
		return mMessage;
	}

	char *stealMessage(void) {
		char *s = mMessage;
		mMessage = nullptr;
		return s;
	}

	// for debugging convience, senes a message to the 
	// console and debug stream
	void print(void);

  private:

	int 	mCode;
	char 	*mMessage;

};





