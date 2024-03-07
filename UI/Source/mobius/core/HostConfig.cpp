/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Model for describing the idiosyncrasies of plugin hosts.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../util/Util.h"
//#include "XmlModel.h"
//#include "XmlBuffer.h"
//#include "XomParser.h"

#include "HostConfig.h"

/****************************************************************************
 *                                                                          *
 *                                HOST CONFIG                               *
 *                                                                          *
 ****************************************************************************/

HostConfig::HostConfig()
{
    init();
}

void HostConfig::init()
{
    mNext               = NULL;
    mVendor             = NULL;
    mProduct            = NULL;
    mVersion            = NULL;
    mStereo             = false;
    mRewindsOnResume    = false;
    mPpqPosTransport    = false;
    mSamplePosTransport = false;
}

HostConfig::~HostConfig()
{
	HostConfig *el, *next;

    delete mVendor;
    delete mProduct;
    delete mVersion;

	for (el = mNext ; el != NULL ; el = next) {
		next = el->getNext();
		el->setNext(NULL);
		delete el;
	}
}

HostConfig* HostConfig::getNext()
{
    return mNext;
}

void HostConfig::setNext(HostConfig* c)
{
    mNext = c;
}

//
// Scope
//

const char* HostConfig::getVendor()
{
    return mVendor;
}

void HostConfig::setVendor(const char* s)
{   
    delete mVendor;
    mVendor = CopyString(s);
}

const char* HostConfig::getProduct()
{
    return mProduct;
}

void HostConfig::setProduct(const char* s)
{   
    delete mProduct;
    mProduct = CopyString(s);
}

const char* HostConfig::getVersion()
{
    return mVersion;
}

void HostConfig::setVersion(const char* s)
{   
    delete mVersion;
    mVersion = CopyString(s);
}

//
// Options
//

bool HostConfig::isStereo()
{
    return mStereo;
}

void HostConfig::setStereo(bool b)
{
    mStereo = b;
}

bool HostConfig::isRewindsOnResume()
{
    return mRewindsOnResume;
}

void HostConfig::setRewindsOnResume(bool b)
{
    mRewindsOnResume = b;
}

bool HostConfig::isPpqPosTransport()
{
    return mPpqPosTransport;
}

void HostConfig::setPpqPosTransport(bool b)
{
    mPpqPosTransport = b;
}

bool HostConfig::isSamplePosTransport()
{
    return mSamplePosTransport;
}

void HostConfig::setSamplePosTransport(bool b)
{
    mSamplePosTransport = b;
}

/****************************************************************************
 *                                                                          *
 *                                HOST CONFIGS                              *
 *                                                                          *
 ****************************************************************************/

HostConfigs::HostConfigs()
{
    init();
}

void HostConfigs::init()
{
    mConfigs = NULL;
    mVendor = NULL;
    mProduct = NULL;
    mVersion = NULL;
}
    
HostConfigs::~HostConfigs()
{
    delete mConfigs;
    delete mVendor;
    delete mProduct;
    delete mVersion;
}

HostConfig *HostConfigs::getConfigs()
{
    return mConfigs;
}

void HostConfigs::setConfigs(HostConfig* configs)
{
    mConfigs = configs;
}

void HostConfigs::setHost(const char* vendor, const char* product, 
                                 const char* version)
{
    delete mVendor;
    delete mProduct;
    delete mVersion;

    // collapse empty strings to NULL for our comparisons
    mVendor = copyString(vendor);
    mProduct = copyString(product);
    mVersion = copyString(version);
}

/**
 * Collapse empty strings to NULL, so the HostInterface can give
 * us static buffers which may be empty.
 */
char* HostConfigs::copyString(const char* src)
{
    char* copy = NULL;
    if (src != NULL && strlen(src) > 0)
      copy = CopyString(src);
    return copy;
}

void HostConfigs::add(HostConfig* c) 
{
	// keep them ordered
	HostConfig *prev;
	for (prev = mConfigs ; prev != NULL && prev->getNext() != NULL ; 
		 prev = prev->getNext());

	if (prev == NULL)
	  mConfigs = c;
	else
	  prev->setNext(c);
}

/**
 * Find the most specific configuration for the currently scoped host.
 * The notion here is that there can be a HostConfig with no vendor
 * to represent the default options, one with just a vendor 
 * for everything from one company (unlikely), one with just a product
 * for all versions of a product, and one with a product and a version
 * for a specific version.  Version is relevant only if product is non-null.
 *
 * I was originally thinking that each option could have a default
 * and be overridden by more specific configs but because we're dealing
 * with bools, there is no "unset" state so we'll find the most specific
 * config for the host and use everything in it.  Since we're only dealing
 * with three flags that isn't so bad, but it also means we can't have
 * a global override.  I suppose we could make the host-less config
 * be a global override rather than a fallback default.
 * 
 */
HostConfig* HostConfigs::getConfig()
{
    HostConfig* found = NULL;

    for (HostConfig* c = mConfigs ; c != NULL ; c = c->getNext()) {
        
        if (isMatch(c) && (found == NULL || isMoreSpecific(found, c)))
          found = c;
    }

    return found;
}

/**
 * If any of the search fields is NULL then it can only match
 * a HostConfig that has a NULL value.  In practice this should 
 * only happen for Vendor (does AU give us that?) and Version.
 * VSTs should provide all three.  
 *
 * If a search field is non-NULL it will match a config if the
 * values are the same or the config value is NULL.  This lets
 * configs have "wildcard" values.  For example, to configure
 * all versions of Cubase just set the product and leave the vendor
 * and version blank.  In fact, I expect that vendor will be missing
 * most of the time.
 */
bool HostConfigs::isMatch(HostConfig* config)
{
    return ((config->getVendor() == NULL ||
             StringEqual(mVendor, config->getVendor())) &&

            (config->getProduct() == NULL ||
             StringEqual(mProduct, config->getProduct())) &&

            (config->getVersion() == NULL ||
             StringEqual(mVersion, config->getVersion()))
        );
}

/**
 * One config is more specific than another if it has a non-null value
 * for any of the fields and the previous one has a NULL value.
 */
bool HostConfigs::isMoreSpecific(HostConfig* prev, HostConfig* neu)
{
    return ((prev->getVendor() == NULL && neu->getVendor() != NULL) ||
            (prev->getProduct() == NULL && neu->getProduct() != NULL) ||
            (prev->getVersion() == NULL && neu->getVersion() != NULL));
}

bool HostConfigs::isStereo()
{
    HostConfig* config = getConfig();
    return (config != NULL) ? config->isStereo() : false;
}

bool HostConfigs::isRewindsOnResume()
{
    HostConfig* config = getConfig();
    return (config != NULL) ? config->isRewindsOnResume() : false;
}

bool HostConfigs::isPpqPosTransport()
{
    HostConfig* config = getConfig();
    return (config != NULL) ? config->isPpqPosTransport() : false;
}

bool HostConfigs::isSamplePosTransport()
{
    HostConfig* config = getConfig();
    return (config != NULL) ? config->isSamplePosTransport() : false;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
