/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * An simple file-based message catlog.
 * 
 * NOTE: This is an old attempt at allowing Mobius to have localizable
 * text in the UI.  I'm abandoning that but we still need to keep this
 * around as the older code is rewritten.
 *
 */

#ifndef UTIL_MSGCAT_H
#define UTIL_MSGCAT_H

#include <stdio.h>

class MessageCatalog {

  public:
	
	MessageCatalog();
	MessageCatalog(const char* file);
	~MessageCatalog();

	bool read(const char* file);

	const char* get(int index);

	void dump();

  private:

	void init();
	void clear();
	int read(FILE* fp, char** messages);

	char** mMessages;
	int mMessageCount;

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#endif
