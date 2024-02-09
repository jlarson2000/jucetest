/*
 * Copyright (c) 2010 Jeffrey S. Larson  <jeff@circularlabs.com>
 * All rights reserved.
 * See the LICENSE file for the full copyright and license declaration.
 * 
 * ---------------------------------------------------------------------
 * 
 * Model for persistent UI configuration.
 *
 * This has been reduced from the original UIConfig to think
 * about other ways of doing things.
 *
 */

#pragma once

#include "../util/List.h"

#define DEFAULT_REFRESH_INTERVAL 100
#define DEFAULT_MESSAGE_DURATION 2
#define DEFAULT_ALERT_INTERVALS 10

/**
 * Holds display component locations.
 * This needs to be in the UI model because we can save it
 * with the configuration.  But keep the model generic and independent.
 * We only store names and coordinates.
 */

class Location
{
  public:

	Location();
	Location(const char* name);
	~Location();

	void setName(const char *name);
	void setX(int i);
	void setY(int i);
    void setDisabled(bool b);

	const char* getName();
	int getX();
	int getY();
    bool isDisabled();

  private:

	void init();

	char* mName;
	int mX;
	int mY;
    bool mDisabled;

};

class Button
{
  public:

    Button();
    ~Button();

    const char* getFunction();
    void setFunction();

  private:

    char* function;
};
    
class UIConfig
{
  public:

    UIConfig();
    ~UIConfig();

    void setName(const char* name);

    void setRefreshInterval(int i);
    int getRefreshInterval();

    // not used, what was this for?
    void setAlertIntervals(int i);
    int getAlertIntervals();

    void setMessageDuration(int i);
    int getMessageDuration();

    // captured window size
    // not used yet
    void setWindowWidth(int w) {
        width = w;
    }

    void setWindowHeight(int h) {
        height = h;
    }

    int getWindowWidth() {
        return width;
    }

    int getWindowHeight() {
        return height;
    }

    std::vector<Location>* getLocations();
    void setLocations(std::vector<Location>& src);

    std::vector<Button>* getButtons();
    void setButtons(std::vector<Button>& src);

    StringList* getParameters();
    void setParameters(StringList* l);

    StringList* getFloatingStrip();
    void setFloatingStrip(StringList* l);

    StringList* getFloatingStrip2();
    void setFloatingStrip2(StringList* l);

    StringList* getDockedStrip();
    void setDockedStrip(StringList* l);

    // what is this?
    const char* getError();

  private:

    void init();
    void checkDisplayComponents();

    char mError[256];   // captured parser error
    char* mName;
    int mRefreshInterval;
    int mAlertIntervals;    // unused, what was this for?
    int mMessageDuration;
    int width;
    int height;
    
    std::vector<Location> locations;
    std::vector<Button> buttons;

	StringList* mParameters;
	StringList* mFloatingStrip;
	StringList* mFloatingStrip2;
	StringList* mDockedStrip;

};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
